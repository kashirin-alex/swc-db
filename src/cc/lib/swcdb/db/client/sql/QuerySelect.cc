/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Time.h"
#include "swcdb/core/config/Property.h"
#include "swcdb/db/client/sql/QuerySelect.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC { namespace client { namespace SQL {

namespace {

  static const char     TOKEN_SELECT[] = "select";
  static const uint8_t  LEN_SELECT = 6;
  static const char     TOKEN_DUMP[] = "dump";
  static const uint8_t  LEN_DUMP = 4;
  static const char     TOKEN_WHERE[] = "where";
  static const uint8_t  LEN_WHERE = 5;
  static const char     TOKEN_COL[] = "col";
  static const uint8_t  LEN_COL = 3;
  static const char     TOKEN_CELLS[] = "cells";
  static const uint8_t  LEN_CELLS = 5;


  static const char     TOKEN_RANGE[] = "range";
  static const uint8_t  LEN_RANGE = 5;
  static const char     TOKEN_KEY[] = "key";
  static const uint8_t  LEN_KEY = 3;
  static const char     TOKEN_VALUE[] = "value";
  static const uint8_t  LEN_VALUE = 5;
  static const char     TOKEN_TIMESTAMP[] = "timestamp";
  static const uint8_t  LEN_TIMESTAMP = 9;
  static const char     TOKEN_OFFSET_KEY[] = "offset_key";
  static const uint8_t  LEN_OFFSET_KEY = 10;
  static const char     TOKEN_OFFSET_REV[] = "offset_rev";
  static const uint8_t  LEN_OFFSET_REV = 10;
  static const char     TOKEN_RANGE_END_REST[] = "range_end_rest";
  static const uint8_t  LEN_RANGE_END_REST = 14;

  static const char     TOKEN_LIMIT[] = "limit";
  static const uint8_t  LEN_LIMIT = 5;
  static const char     TOKEN_OFFSET[] = "offset";
  static const uint8_t  LEN_OFFSET = 6;
  static const char     TOKEN_MAX_VERS[] = "max_versions";
  static const uint8_t  LEN_MAX_VERS = 12;
  static const char     TOKEN_MAX_BUFF[] = "max_buffer";
  static const uint8_t  LEN_MAX_BUFF = 10;
  static const char     TOKEN_ONLY_DELETES[] = "only_deletes";
  static const uint8_t  LEN_ONLY_DELETES = 12;
  static const char     TOKEN_ONLY_KEYS[] = "only_keys";
  static const uint8_t  LEN_ONLY_KEYS = 9;

}

QuerySelect::QuerySelect(const Clients::Ptr& a_clients,
                         const std::string& a_sql, DB::Specs::Scan& a_specs,
                         std::string& a_message)
                        : Reader(a_sql, a_message),
                          clients(a_clients), specs(a_specs) {
}

int QuerySelect::parse_select() {

  bool token_cmd = false;
  bool token_where = false;

  while(remain && !err && (!token_cmd || !token_where)) {
    if(!token_cmd) {
      expect_token(TOKEN_SELECT, LEN_SELECT, token_cmd);
      continue;
    }
    if(!token_where) {
      expect_token(TOKEN_WHERE, LEN_WHERE, token_where);
      continue;
    }
    --remain;
    ++ptr;
  }

  if(remain && !err)
    read_columns_intervals();

  if(remain && !err) {
    read_flags(specs.flags);
    if(specs.flags.was_set) {
      // apply global-scope flags to cells_intervals
      for(auto& col : specs.columns) {
        for(auto& intval : col) {
          if(!intval->flags.was_set)
            intval->flags.copy(specs.flags);
        }
      }
    }
  }
  return err;
}

int QuerySelect::parse_dump(std::string& fs, std::string& filepath,
                            uint64_t& split_size,
                            std::string& ext, int& level) {
  bool token_cmd = false;
  bool token_col = false;

  seek_space();
  expect_token(TOKEN_DUMP, LEN_DUMP, token_cmd);
  if(err)
    return err;
  seek_space();
  expect_token(TOKEN_COL, LEN_COL, token_col);
  if(err)
    return err;

  expect_eq();
  if(err)
    return err;
  std::string buff;
  read(buff);
  if(!err && buff.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing col 'id|name'");
  if(err)
    return err;
  auto schema = add_column(buff);
  if(err)
    return err;

  bool into = false;
  seek_space();
  expect_token("into", 4, into);
  if(err)
    return err;


  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("fs", 2))) {
      expect_eq();
      if(err)
        break;
      read(fs);
      if(!err && fs.empty())
        error_msg(Error::SQL_PARSE_ERROR, "missing 'fs' value");
      continue;
    }

    if((any = found_token("path", 4))) {
      expect_eq();
      if(err)
        break;
      read(filepath);
      continue;
    }

    if((any = found_token("split", 5))) {
      expect_eq();
      if(err)
        break;
      std::string n;
      read(n);
      if(!err && n.empty())
        error_msg(Error::SQL_PARSE_ERROR, "missing 'split' value");
      if(err)
        break;
      try {
        int64_t v;
        Config::Property::from_string(n, &v);
        split_size = v;
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error_msg(e.code(), e.what());
      }
      continue;
    }

    if((any = found_token("ext", 3))) {
      expect_eq();
      if(err)
        break;
      read(ext);
      if(!err && ext.empty())
        error_msg(Error::SQL_PARSE_ERROR, "missing 'ext' value");
      continue;
    }

    if((any = found_token("level", 5))) {
      expect_eq();
      if(err)
        break;
      uint32_t n;
      bool was_set;
      read_uint32_t(n, was_set);
      if(!err)
        level = n;
      continue;
    }
  }
  if(!err && filepath.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing 'path'");
  if(err)
    return err;

  seek_space();
  if(found_token(TOKEN_WHERE, LEN_WHERE)) {
    DB::SchemasVec cols(1, schema);
    read_cells_intervals(cols);

    for(auto& col : specs.columns)
      if(!col.size()) {
        error_msg(Error::SQL_PARSE_ERROR, "missing cells-intervals");
        return err;
      }

  } else {
    for(auto& col : specs.columns)
      col.emplace_back(DB::Specs::Interval::make_ptr());
  }

  return err;
}

void QuerySelect::parse_output_flags(uint8_t& output_flags) {
  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("OUTPUT_NO_TS", 12))) {
      output_flags |= DB::OutputFlag::NO_TS;
      continue;
    }
    if((any = found_token("OUTPUT_NO_VALUE", 15))) {
      output_flags |= DB::OutputFlag::NO_VALUE;
      continue;
    }
    if((any = found_token("OUTPUT_NO_ENCODE", 16))) {
      output_flags |= DB::OutputFlag::NO_ENCODE;
      continue;
    }
  }
}

void QuerySelect::parse_display_flags(uint8_t& display_flags) {

  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("DISPLAY_TIMESTAMP", 17))) {
      display_flags |= DB::DisplayFlag::TIMESTAMP;
      continue;
    }
    if((any = found_token("DISPLAY_DATETIME", 16))) {
      display_flags |= DB::DisplayFlag::DATETIME;
      continue;
    }
    if((any = found_token("DISPLAY_BINARY", 14))) {
      display_flags |= DB::DisplayFlag::BINARY;
      continue;
    }
    if((any = found_token("DISPLAY_SPECS", 13))) {
      display_flags |= DB::DisplayFlag::SPECS;
      continue;
    }
    if((any = found_token("DISPLAY_STATS", 13))) {
      display_flags |= DB::DisplayFlag::STATS;
      continue;
    }
    if((any = found_token("DISPLAY_COLUMN", 14))) {
      display_flags |= DB::DisplayFlag::COLUMN;
      continue;
    }
  }
}

void QuerySelect::read_columns_intervals() {
  bool token_col = false;
  bool bracket_round = false;
  bool eq = false;
  bool col_names_set = false;
  bool processed = false;
  bool possible_and = false;
  DB::Schemas::SelectorPatterns schema_patterns;

  std::string col_name;
  DB::SchemasVec cols;

  while(remain && !err) {
    if(found_space())
      continue;
    if(possible_and) {
      if(found_token(TOKEN_AND, LEN_AND))
        possible_and = false;
      else
        break;
    }

    if(!token_col) {
      expect_token(TOKEN_COL, LEN_COL, token_col);
      continue;
    }

    if(token_col && !col_names_set) {
      // "col(, 1 2 ,re"aa$, =^"Ds", "3 4", tags>=['1',>4,<'5'])"
      //   --> ["1","2", [patterns], "3 4", [tags]]
      if(found_space())
        continue;

      if(!bracket_round) {
        expect_token("(", 1, bracket_round);
        continue;
      }

      if(found_char(','))
        continue;

      if(found_char(')')) {
        if(!schema_patterns.names.empty() ||
            schema_patterns.tags.comp != Condition::NONE) {
          add_column(schema_patterns, cols);
          schema_patterns.names.clear();
          schema_patterns.tags.clear();
        }

        if(cols.empty()) {
          error_msg(Error::SQL_PARSE_ERROR, "missing col 'id|name'");
          break;
        }
        bracket_round = false;
        col_names_set = true;
        continue;

      } else if(remain <= 1) {
        error_msg(Error::SQL_PARSE_ERROR, "missing ')'");
        break;
      }

      if(found_token("tags", 4)) {
        read_column_tags(schema_patterns.tags);
        continue;
      }

      read_column(",)", col_name, schema_patterns.names);
      if(!col_name.empty()) {
        cols.push_back(add_column(col_name));
        col_name.clear();
      }
      continue;
    }

    if(col_names_set) {

      if(!processed) {

        if(!eq) {
          expect_token("=", 1, eq);
          continue;
        }
        if(!bracket_round) {
          expect_token("(", 1, bracket_round);
          continue;
        }

        read_cells_intervals(cols);
        cols.clear();

        bracket_round = false;
        eq = false;
        processed = true;
        continue;
      }

      if(!bracket_round) {
        expect_token(")", 1, bracket_round);
        continue;
      }

      col_names_set = false;
      bracket_round = false;
      processed = false;
      token_col = false;
      possible_and = true;
    }

  }

}

DB::Schema::Ptr QuerySelect::add_column(const std::string& col_str) {
  auto schema = get_schema(clients, col_str);
  if(err)
    return nullptr;
  for(auto& col : specs.columns) {
    if(schema->cid == col.cid)
      return schema;
  }
  specs.columns.emplace_back(schema->cid);
  return schema;
}

void QuerySelect::add_column(const DB::Schemas::SelectorPatterns& patterns,
                             DB::SchemasVec& cols) {
  auto schemas = get_schema(clients, patterns);
  if(err)
  return;

  for(auto& schema : schemas) {
    auto it = specs.columns.cbegin();
    for(; it != specs.columns.cend(); ++it) {
      if(schema->cid == it->cid)
        break;
    }
    if(it == specs.columns.cend())
      specs.columns.emplace_back(schema->cid);
    cols.push_back(schema);
  }
}

void QuerySelect::read_cells_intervals(const DB::SchemasVec& cols) {
  bool token_cells = false;
  bool bracket_round = false;
  bool eq = false;
  bool processed = false;
  bool possible_and = false;

  while(remain && !err) {
    if(possible_and) {
      if(found_space())
        continue;
      if(found_token(TOKEN_AND, LEN_AND))
        possible_and = false;
      else
        break;
    }
    if(!token_cells) {
      expect_token(TOKEN_CELLS, LEN_CELLS, token_cells);
      continue;
    }

    if(!processed) {

      if(!eq) {
        expect_token("=", 1, eq);
        continue;
      }
      if(!bracket_round) {
        expect_token("(", 1, bracket_round);
        continue;
      }

      bool value_except = false;
      auto col_type = cols.front()->col_type;
      for(auto& schema : cols) {
        if(col_type != schema->col_type) {
          value_except = true;
          break;
        }
      }
      auto spec = DB::Specs::Interval::make_ptr(col_type);
      read_cells_interval(*spec.get(), value_except);

      for(auto& col : specs.columns) {
        for(auto& schema : cols) {
          if(col.cid == schema->cid)
            col.emplace_back(DB::Specs::Interval::make_ptr(*spec.get()));
        }
      }

      bracket_round = false;
      eq = false;
      processed = true;
      continue;
    }

    if(!bracket_round) {
      expect_token(")", 1, bracket_round);
      continue;
    }
    bracket_round = false;
    processed = false;
    token_cells = false;
    possible_and = true;
  }

}

void QuerySelect::read_cells_interval(DB::Specs::Interval& spec,
                                    bool value_except) {

  uint32_t escape = 0;
  bool quote_1 = false;
  bool quote_2 = false;
  bool is_quoted = false;
  bool possible_and = false;
  bool found_any = false;
  bool flw_and = false;
  uint32_t base_remain = remain;
  const char* base_rptr = ptr;

  while(remain && !err) {

    if(possible_and) {
      found_any = true;
      if(found_space())
        continue;
      if(found_token(TOKEN_AND, LEN_AND)) {
        possible_and = false;
        flw_and = true;
      } else
        break;
    }

    if(!escape && found_char('\\')) {
      escape = remain;
      continue;
    } else if(escape && escape != remain)
      escape = 0;

    if(!escape) {
      if(((!is_quoted || quote_1) && found_quote_single(quote_1)) ||
         ((!is_quoted || quote_2) && found_quote_double(quote_2))) {
        is_quoted = quote_1 || quote_2;
        continue;
      }
      if(found_space())
        continue;
    }

    if(is_quoted) {
      ++ptr;
      --remain;
      continue;
    }

    if(found_token(TOKEN_RANGE, LEN_RANGE)) {
      read_range(spec.range_begin, spec.range_end, flw_and);
      while(found_space());
      if(found_token(TOKEN_RANGE_END_REST, LEN_RANGE_END_REST))
        spec.set_opt__range_end_rest();
      possible_and = true;
      continue;
    }

    if(!found_token(TOKEN_ONLY_KEYS, LEN_ONLY_KEYS) &&
        found_token(TOKEN_KEY, LEN_KEY)) {
      auto& key_intval = spec.key_intervals.add();
      read_key(key_intval.start, key_intval.finish, flw_and, spec.options);
      possible_and = true;
      continue;
    }

    if(found_token(TOKEN_TIMESTAMP, LEN_TIMESTAMP)) {
      read_timestamp(spec.ts_start, spec.ts_finish, flw_and);
      possible_and = true;
      continue;
    }

    if(found_token(TOKEN_VALUE, LEN_VALUE)) {
      if(value_except)
        return error_msg(
          Error::SQL_PARSE_ERROR, "Value require the same columns type");
      read_value(spec.values.col_type, spec.values.add());
      possible_and = true;
      continue;
    }

    if(found_token(TOKEN_OFFSET_KEY, LEN_OFFSET_KEY)) {
      expect_eq();
      Reader::read_key(spec.offset_key);
      possible_and = true;
      continue;
    }

    if(found_token(TOKEN_OFFSET_REV, LEN_OFFSET_REV)) {
      expect_eq();
      std::string buf;
      read(buf);
      if(err)
        return;
      spec.offset_rev = Time::parse_ns(err, buf);
      if(err) {
        error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
        return;
      }
      possible_and = true;
      continue;
    }

    if(*ptr == ')')
      break;
    ++ptr;
    --remain;

  }

  if(!found_any) {
    remain = base_remain;
    ptr = base_rptr;
  }
  read_flags(spec.flags);
}


void QuerySelect::read_range(DB::Cell::Key& begin, DB::Cell::Key& end,
                           bool flw) {
  uint32_t base_remain = remain;
  const char* base_ptr = ptr;

  Condition::Comp comp_right;
  expect_comparator(comp_right);
  if(comp_right != Condition::GE && comp_right != Condition::LE) {
    error_msg(
      Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
    return;
  }

  if(comp_right == Condition::GE)
    Reader::read_key(begin);
  else
    Reader::read_key(end);
  if(err)
    return;

  uint32_t mark_remain = remain;
  const char* mark_ptr = ptr;

  uint32_t remain_start = sql.length()+1;

  ptr = base_ptr - LEN_RANGE;
  for(remain = 0; remain++ < remain_start; ) {
    --ptr;
    if(flw) {
      if(found_token(TOKEN_AND, LEN_AND))
        break;
    } else if(found_char('('))
      break;
  }

  seek_space();
  if(*ptr != '[') {
    remain = mark_remain;
    ptr = mark_ptr;
    return;
  }

  if(comp_right == Condition::GE)
    Reader::read_key(end);
  else
    Reader::read_key(begin);
  if(err) {
    remain = mark_remain;
    ptr = mark_ptr;
    return;
  }

  Condition::Comp comp_left = Condition::NONE;
  expect_comparator(comp_left);
  remain += base_remain;
  if(comp_left != Condition::GE && comp_left != Condition::LE) {
    error_msg(
      Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
    return;
  }
  comp_left = comp_left == Condition::GE ? Condition::LE : Condition::GE;
  if(comp_left == comp_right) {
    error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
    return;
  }

  remain = mark_remain;
  ptr = mark_ptr;
}

void QuerySelect::read_key(DB::Specs::Key& start, DB::Specs::Key& finish,
                         bool flw, uint8_t& options) {
  uint32_t base_remain = remain;
  const char* base_ptr = ptr;

  Condition::Comp comp_right;
  expect_comparator(comp_right);
  if(comp_right != Condition::EQ &&
     comp_right != Condition::GE && comp_right != Condition::LE) {
    error_msg(
      Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed EQ|GE|LE");
    return;
  }

  if(comp_right == Condition::EQ || comp_right == Condition::GE)
    read_key(start);
  else
    read_key(finish);

  if(err)
    return;
  if(comp_right == Condition::EQ) {
    options |= DB::Specs::Interval::OPT_KEY_EQUAL;
    return;
  }

  uint32_t mark_remain = remain;
  const char* mark_ptr = ptr;

  uint32_t remain_start = sql.length()+1;

  ptr = base_ptr - LEN_KEY;
  for(remain = 0; remain++ < remain_start; ) {
    --ptr;
    if(flw) {
      if(found_token(TOKEN_AND, LEN_AND))
        break;
    } else if(found_char('('))
      break;
  }

  seek_space();
  if(*ptr != '[') {
    remain = mark_remain;
    ptr = mark_ptr;
    return;
  }

  if(comp_right == Condition::GE)
    read_key(finish);
  else
    read_key(start);
  if(err) {
    remain = mark_remain;
    ptr = mark_ptr;
    return;
  }

  Condition::Comp comp_left = Condition::NONE;
  expect_comparator(comp_left);
  remain += base_remain;
  if(comp_left != Condition::GE && comp_left != Condition::LE) {
    error_msg(
      Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
    return;
  }
  comp_left = comp_left == Condition::GE ? Condition::LE : Condition::GE;
  if(comp_right == comp_left) {
    error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
    return;
  }

  remain = mark_remain;
  ptr = mark_ptr;
}

void QuerySelect::read_key(DB::Specs::Key& key) {

  bool bracket_square = false;

  seek_space();
  expect_token("[", 1, bracket_square);

  Condition::Comp comp = Condition::NONE;
  std::string fraction;
  while(remain && !err) {
    if(found_space())
      continue;

    found_comparator(comp);
    if(comp == Condition::NONE)
      comp = Condition::EQ;
    read(fraction, ",]");
    key.add(std::move(fraction), comp);

    seek_space();
    if(found_char(','))
      continue;
    break;
  }

  expect_token("]", 1, bracket_square);
}

void QuerySelect::read_value(DB::Types::Column col_type,
                           DB::Specs::Value& value) {
  switch(col_type) {
    case DB::Types::Column::SERIAL: {
      found_comparator(value.comp, false);
      if(value.comp == Condition::NONE)
        value.comp = Condition::EQ;
      else if(value.comp != Condition::EQ && value.comp != Condition::NE)
        return error_msg(Error::SQL_PARSE_ERROR,
          "unsupported 'comparator' allowed EQ, NE");

      bool bracket_square = false;
      bool was_set;
      seek_space();
      expect_token("[", 1, bracket_square);
      if(err)
        return;

      DB::Specs::Serial::Value::Fields fields;
      do {
        uint32_t fid;
        read_uint32_t(fid, was_set, ":");
        if(err)
          return;
        seek_space();
        expect_token(":", 1, was_set);
        if(err)
          return;

        DB::Cell::Serial::Value::Type typ = read_serial_value_type();
        if(err)
          return;
        switch(typ) {

          case DB::Specs::Serial::Value::Type::INT64: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, false);
            if(!is_numeric_comparator(comp))
              return;
            int64_t v;
            read_int64_t(v, was_set, ",]");
            if(err)
              return;
            fields.add(
              DB::Specs::Serial::Value::Field_INT64::make(
                fid, comp, v));
            break;
          }

          case DB::Specs::Serial::Value::Type::DOUBLE: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, false);
            if(!is_numeric_comparator(comp, true))
              return;
            long double v;
            read_double_t(v, was_set, ",]");
            if(err)
              return;
            fields.add(
              DB::Specs::Serial::Value::Field_DOUBLE::make(
                fid, comp, v));
            break;
          }

          case DB::Specs::Serial::Value::Type::BYTES: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, true);
            if(comp == Condition::NONE)
              comp = Condition::EQ;
            std::string buf;
            read(buf, ",]", comp == Condition::RE);
            if(err)
              return;
            fields.add(
              DB::Specs::Serial::Value::Field_BYTES::make(fid, comp, buf));
            break;
          }

          case DB::Specs::Serial::Value::Type::KEY: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, false);
            if(comp == Condition::NONE)
              comp = Condition::EQ;
            else if(comp != Condition::EQ)
              return error_msg(
                Error::SQL_PARSE_ERROR,
                "unsupported 'comparator' allowed EQ");
            std::string buf;
            read(buf, "[");
            if(err)
              return;
            auto seq = DB::Types::range_seq_from(buf);
            if(seq == DB::Types::KeySeq::UNKNOWN)
              return error_msg(Error::SQL_PARSE_ERROR, "bad 'KeqSeq'");
            auto field = DB::Specs::Serial::Value::Field_KEY::make(fid, seq);
            read_key(field->key);
            if(err)
              return;
            fields.add(std::move(field));
            break;
          }

          case DB::Specs::Serial::Value::Type::LIST_INT64: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, false);
            if(comp == Condition::NONE)
              comp = Condition::EQ;
            else if(comp == Condition::RE || comp == Condition::PF)
              return error_msg(
                Error::SQL_PARSE_ERROR, "unsupported PF,RE 'comparator'");
            seek_space();
            expect_token("[", 1, bracket_square);
            if(err)
              return;
            auto field = DB::Specs::Serial::Value::Field_LIST_INT64::make(
              fid, comp);
            do {
              auto& item = field->items.emplace_back();
              found_comparator(item.comp = Condition::NONE, false);
              if(!is_numeric_comparator(item.comp))
                return;
              std::string buff;
              read(buff, ",]");
              if(buff.empty()) {
                field->items.pop_back();
              } else {
                try {
                  item.value = std::stoll(buff);
                } catch(...) {
                  error_msg(Error::SQL_PARSE_ERROR,
                    " signed 64-bit integer out of range");
                }
              }
              seek_space();
            } while(found_char(','));

            expect_token("]", 1, bracket_square);
            if(err)
              return;
            fields.add(std::move(field));
            break;
          }

          case DB::Specs::Serial::Value::Type::LIST_BYTES: {
            Condition::Comp comp = Condition::NONE;
            found_comparator(comp, true);
            if(comp == Condition::NONE)
              comp = Condition::EQ;
            seek_space();
            expect_token("[", 1, bracket_square);
            if(err)
              return;
            auto field = DB::Specs::Serial::Value::Field_LIST_BYTES::make(
              fid, comp);
            do {
              auto& item = field->items.emplace_back();
              found_comparator(item.comp = Condition::NONE, false);
              read(item.value, ",]", item.comp == Condition::RE);
              if(err)
                return;
              if(item.comp == Condition::NONE) {
                if(item.value.empty())
                  field->items.pop_back();
                else
                  item.comp = Condition::EQ;
              }
              seek_space();
            } while(found_char(','));

            expect_token("]", 1, bracket_square);
            if(err)
              return;
            fields.add(std::move(field));
            break;
          }

          default: {
            return error_msg(
              Error::SQL_PARSE_ERROR, "Not Supported Serial Value Type");
          }
        }
        seek_space();
      } while(found_char(','));

      expect_token("]", 1, bracket_square);
      while(remain && !err && (found_char(' ') || found_char('\t')));
      if(err)
        return;

      fields.encode(value);
      break;
    }
    case DB::Types::Column::COUNTER_I64:
    case DB::Types::Column::COUNTER_I32:
    case DB::Types::Column::COUNTER_I16:
    case DB::Types::Column::COUNTER_I8: {
      found_comparator(value.comp, false);
      if(!is_numeric_comparator(value.comp))
        return;
      std::string buf;
      read(buf, nullptr, false);
      if(!err)
        value.set(buf, value.comp);
      break;
    }
    default: {
      found_comparator(value.comp, true);
      if(value.comp == Condition::NONE)
        value.comp = Condition::EQ;
      std::string buf;
      read(buf, nullptr, value.comp == Condition::RE);
      if(!err)
        value.set(buf, value.comp);
      break;
    }
  }
}

void QuerySelect::read_timestamp(DB::Specs::Timestamp& start,
                               DB::Specs::Timestamp& finish, bool flw) {
  uint32_t base_remain = remain;
  const char* base_ptr = ptr;

  Condition::Comp comp_right;
  expect_comparator(comp_right);
  if(comp_right != Condition::EQ &&
     comp_right != Condition::NE &&
     comp_right != Condition::GE &&
     comp_right != Condition::GT &&
     comp_right != Condition::LE &&
     comp_right != Condition::LT) {
    error_msg(
      Error::SQL_PARSE_ERROR,
      "unsupported 'comparator' allowed NE|EQ|GE|GT|LE|LT"
    );
    return;
  }

  std::string buf;
  read(buf);
  if(err)
    return;
  int64_t ts = Time::parse_ns(err, buf);
  if(err) {
    error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
    return;
  }

  if(comp_right == Condition::EQ || comp_right == Condition::NE ||
     comp_right == Condition::GE || comp_right == Condition::GT)
    start.set(ts, comp_right);
  else
    finish.set(ts, comp_right);

  if(comp_right == Condition::EQ || comp_right == Condition::NE)
    return;

  uint32_t mark_remain = remain;
  const char* mark_ptr = ptr;

  uint32_t end = sql.length()+1;

  ptr = base_ptr - LEN_TIMESTAMP;
  for(remain = 0; remain < end; ) {
    ++remain;
    --ptr;
    if(flw) {
      if(found_token(TOKEN_AND, LEN_AND))
        break;
    } else if(found_char('('))
      break;
  }

  buf.clear();
  read(buf);
  if(buf.empty()) {
    remain = mark_remain;
    ptr = mark_ptr;
    return;
  }
  if(err)
    return;
  ts = Time::parse_ns(err, buf);
  if(err) {
    error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
    return;
  }

  Condition::Comp comp_left = Condition::NONE;
  expect_comparator(comp_left);
  remain += base_remain;
  if(comp_left != Condition::GE &&
     comp_left != Condition::GT &&
     comp_left != Condition::LE &&
     comp_left != Condition::LT) {
    error_msg(
      Error::SQL_PARSE_ERROR,
      "unsupported 'comparator' allowed GE|GT|LE|LT"
    );
    return;
  }

  comp_left = comp_left == Condition::GE ? Condition::LE
      : (comp_left == Condition::GT ? Condition::LT
      : (comp_left == Condition::LE ? Condition::GE
      : (comp_left == Condition::LT ? Condition::GT : comp_left)));

  if(comp_left == comp_right) {
    error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
    return;
  }

  if(finish.was_set)
    start.set(ts, comp_left);
  else
    finish.set(ts, comp_left);

  remain = mark_remain;
  ptr = mark_ptr;
}

void QuerySelect::read_flags(DB::Specs::Flags& flags) {

  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token(TOKEN_LIMIT, LEN_LIMIT))) {
      expect_eq();
      read_uint64_t(flags.limit, flags.was_set);
      continue;
    }
    if((any = found_token(TOKEN_OFFSET, LEN_OFFSET))) {
      expect_eq();
      read_uint64_t(flags.offset, flags.was_set);
      continue;
    }
    if((any = found_token(TOKEN_MAX_VERS, LEN_MAX_VERS))) {
      expect_eq();
      read_uint32_t(flags.max_versions, flags.was_set);
      continue;
    }
    if((any = found_token(TOKEN_MAX_BUFF, LEN_MAX_BUFF))) {
      expect_eq();
      read_uint32_t(flags.max_buffer, flags.was_set);
      continue;
    }

    if((any = found_token(TOKEN_ONLY_DELETES, LEN_ONLY_DELETES))) {
      flags.set_only_deletes();
      flags.was_set = true;
      continue;
    }
    if((any = found_token(TOKEN_ONLY_KEYS, LEN_ONLY_KEYS))) {
      flags.set_only_keys();
      flags.was_set = true;
      continue;
    }
    // + LimitType limit_by, offset_by;
  }
}




}}} // SWC::client:SQL namespace
