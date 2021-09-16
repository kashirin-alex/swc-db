/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/QueryUpdate.h"
#include "swcdb/core/Time.h"


namespace SWC { namespace client { namespace SQL {

namespace {

  static const char     TOKEN_CELL[] = "cell";
  static const uint8_t  LEN_CELL = 4;
  static const char     TOKEN_UPDATE[] = "update";
  static const uint8_t  LEN_UPDATE = 6;
  static const char     TOKEN_LOAD[] = "load";
  static const uint8_t  LEN_LOAD = 4;
}

QueryUpdate::QueryUpdate(
        const std::string& a_sql,
        const Query::Update::Handlers::BaseUnorderedMap::Ptr& a_hdlr,
        std::string& a_message)
        : Reader(a_sql, a_message), hdlr(a_hdlr) {
}

int QueryUpdate::parse_update() {

  bool token_cmd = false;
  while(remain && !err && !token_cmd) {
    if(!token_cmd) {
      expect_token(TOKEN_UPDATE, LEN_UPDATE, token_cmd);
      continue;
    }
  }

  if(remain && !err)
    read_cells();

  return err;
}

int QueryUpdate::parse_load(std::string& fs, std::string& filepath,
                            cid_t& cid) {
  bool token = false;

  seek_space();
  expect_token(TOKEN_LOAD, LEN_LOAD, token);
  if(err)
    return err;

  seek_space();
  expect_token("from", 4, token);
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
  }
  if(!err && filepath.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing 'path'");
  if(err)
    return err;

  seek_space();
  expect_token("into", 4, token);
  if(err)
    return err;

  seek_space();
  expect_token("col", 3, token);
  if(err)
    return err;
  expect_eq();
  if(err)
    return err;

  std::string col;
  read(col);
  if(!err && col.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing col 'id|name'");
  if(err)
    return err;

  auto schema = get_schema(hdlr->clients, col);
  if(!err)
    cid = schema->cid;
  return err;
}

void QueryUpdate::parse_display_flags(uint8_t& display_flags) {

  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("DISPLAY_SPECS", 13))) {
      display_flags |= DB::DisplayFlag::SPECS;
      continue;
    }
    if((any = found_token("DISPLAY_STATS", 13))) {
      display_flags |= DB::DisplayFlag::STATS;
      continue;
    }
  }
}

void QueryUpdate::read_cells() {
  bool possible_and = false;
  bool bracket = false;

  while(remain && !err) {
    if(found_space())
      continue;

    if(possible_and) {
      if(found_token(",", 1)) {
        possible_and = false;
        continue;
      } else
        break;
    }

    expect_token(TOKEN_CELL, LEN_CELL, possible_and);

    if(remain && !err) {

      seek_space();
      expect_token("(", 1, bracket);
      if(err)
        return;

      bool on_fraction = false;
      cid_t cid = DB::Schema::NO_CID;
      DB::Cells::Cell cell;
      read_cell(cid, cell, on_fraction);
      if(err)
        return;

      seek_space();
      expect_token(")", 1, bracket);
      if(err)
        return;
      //if(on_fraction) // not impl.
      //  hdlr->add(cid, cell);
      //else
      hdlr->add(cid, cell);
    }
  }
}

void QueryUpdate::read_cell(cid_t& cid, DB::Cells::Cell& cell,
                          bool& on_fraction) {
  bool comma = false;

  read_flag(cell.flag, on_fraction);
  if(!err)
    expect_comma(comma);
  if(err)
    return;


  std::string col;
  read(col, ",");
  if(!err)
    expect_comma(comma);
  if(err)
    return;
  auto schema = get_schema(hdlr->clients, col);
  if(err)
    return;
  cid = schema->cid;
  hdlr->create(schema);


  read_key(cell.key);

  comma = false;
  bool require_ts = cell.flag == DB::Cells::DELETE_VERSION;
  if(!err && require_ts)
    expect_comma(comma);
  if(err)
    return;

  seek_space();
  if(err || (!comma && !found_char(','))) {
    cell.set_time_order_desc(true);
    return;
  }

  if(cell.flag == DB::Cells::INSERT) {
    bool time_order;
    seek_space();
    if((time_order = found_token("DESC", 4)) ||
       !(time_order = found_token("ASC", 3)))
      cell.set_time_order_desc(true);
    if(time_order) {
      seek_space();
      if(err || !found_char(','))
        return;
    }
  }

  std::string buf;
  read(buf, ",)");
  if(err)
    return;
  if(!buf.empty())
    cell.set_timestamp(Time::parse_ns(err, buf));
  if(err || (require_ts && buf.empty())) {
    error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
    return;
  }
  buf.clear();

  seek_space();
  if(err || !found_char(','))
    return;

  switch(schema->col_type) {
    case DB::Types::Column::PLAIN: {
      std::string value;
      read(value, ",)");
      if(err)
        return;
      seek_space();
      if(err)
        return;
      if(found_char(',')) {
        DB::Types::Encoder enc = read_encoder();
        if(err)
          return;
        cell.set_value(enc, value);
      } else {
        cell.set_value(value, true);
      }
      break;
    }

    case DB::Types::Column::SERIAL: {
      bool bracket_square = false;
      bool was_set;
      seek_space();
      expect_token("[", 1, bracket_square);
      if(err)
        return;

      DB::Cell::Serial::Value::FieldsWriter wfields;
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

          case DB::Cell::Serial::Value::Type::INT64: {
            int64_t v;
            read_int64_t(v, was_set, ",]");
            if(err)
              return;
            wfields.add(fid, v);
            break;
          }

          case DB::Cell::Serial::Value::Type::DOUBLE: {
            long double v;
            read_double_t(v, was_set, ",]");
            if(err)
              return;
            wfields.add(fid, v);
            break;
          }

          case DB::Cell::Serial::Value::Type::BYTES: {
            read(buf, ",]");
            if(err)
              return;
            wfields.add(fid, buf);
            buf.clear();
            break;
          }

          case DB::Cell::Serial::Value::Type::KEY: {
            DB::Cell::Key fkey;
            read_key(fkey);
            if(err)
              return;
            wfields.add(fid, fkey);
            break;
          }

          case DB::Cell::Serial::Value::Type::LIST_INT64: {
            seek_space();
            expect_token("[", 1, bracket_square);
            if(err)
              return;
            Core::Vector<int64_t> items;
            do {
              seek_space();
              if(!is_char(",]")) {
                read_int64_t(items.emplace_back(), was_set, ",]");
                if(err)
                  return;
                seek_space();
              }
            } while(found_char(','));
            expect_token("]", 1, bracket_square);
            if(err)
              return;
            wfields.add(fid, items);
            break;
          }

          case DB::Cell::Serial::Value::Type::LIST_BYTES: {
            seek_space();
            expect_token("[", 1, bracket_square);
            if(err)
              return;
            Core::Vector<std::string> items;
            do {
              seek_space();
              if(!is_char(",]")) {
                read(items.emplace_back(), ",]");
                if(err)
                  return;
                seek_space();
              }
            } while(found_char(','));
            expect_token("]", 1, bracket_square);
            if(err)
              return;
            wfields.add(fid, items);
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

      if(found_char(',')) {
        DB::Types::Encoder enc = read_encoder();
        if(err)
          return;
        cell.set_value(enc, wfields.base, wfields.fill());
      } else {
        cell.set_value(wfields.base, wfields.fill(), true);
      }
      break;
    }

    case DB::Types::Column::COUNTER_I64:
    case DB::Types::Column::COUNTER_I32:
    case DB::Types::Column::COUNTER_I16:
    case DB::Types::Column::COUNTER_I8: {
      std::string value;
      read(value, ",)");
      if(err)
        return;
      const uint8_t* valp = reinterpret_cast<const uint8_t*>(value.c_str());
      size_t _remain = value.length();
      uint8_t op;
      int64_t v;
      op_from(&valp, &_remain, op, v);
      if(err) {
        error_msg(Error::SQL_PARSE_ERROR, Error::get_text(err));
        return;
      }
      cell.set_counter(
        op,
        v,
        op & DB::Cells::OP_EQUAL
          ? schema->col_type
          : DB::Types::Column::COUNTER_I64
      );
      break;
    }
    default:
      break;
  }
}

void QueryUpdate::op_from(const uint8_t** ptrp, size_t* remainp,
                        uint8_t& op, int64_t& value) {
  if(!*remainp) {
    op = DB::Cells::OP_EQUAL;
    value = 0;
    return;
  }
  if(**ptrp == '=') {
    op = DB::Cells::OP_EQUAL;
    ++*ptrp;
    if(!--*remainp) {
      value = 0;
      return;
    }
  } else {
    op = 0;
  }
  const char* p = reinterpret_cast<const char*>(*ptrp);
  char *last = const_cast<char*>(p + (*remainp > 30 ? 30 : *remainp));
  errno = 0;
  value = strtoll(p, &last, 0);
  if(errno) {
    err = errno;
  } else if(last > p) {
    *remainp -= last - p;
    *ptrp = reinterpret_cast<const uint8_t*>(last);
  } else {
    err = EINVAL;
  }
}

void QueryUpdate::read_flag(uint8_t& flag, bool& on_fraction) {
  std::string buf;
  read(buf, ",");
  if(err)
  return;

  switch(buf.length()) {
  case 1: {
    switch(*buf.data()) {
      case '1':
        flag = DB::Cells::INSERT;
        on_fraction = false;
        return;
      case '2':
        flag = DB::Cells::DELETE;
        on_fraction = false;
        return;
      case '3':
        flag = DB::Cells::DELETE_VERSION;
        on_fraction = false;
        return;
      case '4':
        flag = DB::Cells::INSERT;
        on_fraction = true;
        return;
      case '5':
        flag = DB::Cells::DELETE;
        on_fraction = true;
        return;
      case '6':
        flag = DB::Cells::DELETE_VERSION;
        on_fraction = true;
        return;
      default:
        break;
    }
    break;
  }
  case 6: {
    if(Condition::str_case_eq(buf.data(), "INSERT", 6)) {
      flag = DB::Cells::INSERT;
      on_fraction = false;
      return;
    }
    if(Condition::str_case_eq(buf.data(), "DELETE", 6)) {
      flag = DB::Cells::DELETE;
      on_fraction = false;
      return;
    }
    break;
  }
  case 14: {
    if(Condition::str_case_eq(buf.data(), "DELETE_VERSION", 14)) {
      flag = DB::Cells::DELETE_VERSION;
      on_fraction = false;
      return;
    }
    break;
  }
  case 15: {
    if(Condition::str_case_eq(buf.data(), "INSERT_FRACTION", 15)) {
      flag = DB::Cells::INSERT;
      on_fraction = true;
      return;
    }
    if(Condition::str_case_eq(buf.data(), "DELETE_FRACTION", 15)) {
      flag = DB::Cells::DELETE;
      on_fraction = true;
      return;
    }
    break;
  }
  case 23: {
    if(Condition::str_case_eq(buf.data(), "DELETE_FRACTION_VERSION", 23)) {
      flag = DB::Cells::DELETE_VERSION;
      on_fraction = true;
      return;
    }
    break;
  }
  default:
    break;
  }
  std::string s;
  s.reserve(24 + buf.size());
  s.append("unsupported cell flag='");
  s.append(buf);
  s += '\'';
  error_msg(Error::SQL_PARSE_ERROR, s);

}



}}} // SWC::client:SQL namespace
