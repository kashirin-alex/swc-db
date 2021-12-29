/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/client/sql/SQL.h"


namespace SWC { namespace client { namespace SQL {


Reader::Reader(const std::string& a_sql, std::string& a_message)
              : sql(a_sql), message(a_message),
                ptr(sql.data()), remain(sql.length()),
                err(Error::OK) {
}

bool Reader::is_char(const char* stop) const {
  if(stop) do {
    if(*stop++ == *ptr)
      return true;
  } while(*stop);
  return false;
}

bool Reader::found_char(const char c) {
  if(*ptr == c) {
    ++ptr;
    --remain;
    return true;
  }
  return false;
}

bool Reader::found_space() {
  return found_char(' ') || found_char('\t')
      || found_char('\n') || found_char('\r');
}

bool Reader::found_quote_single(bool& quote) {
  if(found_char('\'')) {
    quote = !quote;
    return true;
  }
  return false;
}

bool Reader::found_quote_double(bool& quote) {
  if(found_char('"')) {
    quote = !quote;
    return true;
  }
  return false;
}

bool Reader::found_token(const char* token, uint8_t token_len) {
  if(remain >= token_len && Condition::str_case_eq(ptr, token, token_len)) {
    ptr += token_len;
    remain -= token_len;
    return true;
  }
  return false;
}

bool Reader::found_comparator(Condition::Comp& comp, bool extended) {
  while(remain) {
    if(found_space())
      continue;
    if((comp = Condition::from(&ptr, &remain, extended)) != Condition::NONE)
      return true;
    break;
  }
  return false;
}

void Reader::seek_space() {
  while(remain && !err && found_space());
}

void Reader::expect_eq() {
  seek_space();
  bool eq = false;
  expect_token("=", 1, eq); // ? (not space is eq)
}

void Reader::expect_comma(bool& comma) {
  seek_space();
  expect_token(",", 1, comma);
}

void Reader::expect_comparator(Condition::Comp& comp, bool extended) {
  if(found_comparator(comp, extended) && comp != Condition::NONE)
    return;
  error_msg(Error::SQL_PARSE_ERROR, "missing 'comparator'");
}

void Reader::expect_digit() {
  if(remain >= 1) {
    if(std::isdigit(*ptr)) {
      ++ptr;
      --remain;
      return;
    }
  }
  error_msg(Error::SQL_PARSE_ERROR, "missing 'digit'");
}

void Reader::expected_boolean(bool& value) {
  if(found_char('1') || found_token(TOKEN_BOOL_TRUE, LEN_BOOL_TRUE))
    value = true;
  else if(found_char('0') || found_token(TOKEN_BOOL_FALSE, LEN_BOOL_FALSE))
    value = false;
  else {
    ++ptr;
    --remain;
    error_msg(Error::SQL_PARSE_ERROR, "missing 'bool'");
  }
}

void Reader::expect_token(const char* token, uint8_t token_len, bool& found) {
  if(remain >= token_len) {
    if(found_space())
      return;

    if(Condition::str_case_eq(ptr, token, token_len)) {
      ptr += token_len;
      remain -= token_len;
      found = true;
      return;
    }
  }
  error_msg(Error::SQL_PARSE_ERROR, "missing '"+std::string(token)+"'");
}

DB::Schema::Ptr Reader::get_schema(const Clients::Ptr& clients,
                                   const std::string& col) {
  DB::Schema::Ptr schema;
  if(std::find_if(col.cbegin(), col.cend(),
      [](unsigned char c){ return !std::isdigit(c); } ) != col.cend()){
    schema = clients->get_schema(err, col);
  } else {
    try {
      schema = clients->get_schema(err, std::stoull(col));
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      err = e.code();
    }
  }
  if(err)
    error_msg(err, "problem getting column '"+col+"' schema");
  return schema;
}

DB::SchemasVec
Reader::get_schema(const Clients::Ptr& clients,
                   const DB::Schemas::SelectorPatterns& patterns) {
  auto schemas = clients->get_schema(err, patterns);
  if(err) {
    std::string msg("problem getting columns schema on patterns");
    if(!patterns.names.empty()) {
      msg.append(" names=[");
      for(auto& p : patterns.names) {
        msg.append(Condition::to_string(p.comp, true));
        msg.append("'" + p + "', ");
      }
      msg += ']';
    }
    if(!patterns.tags.empty() || patterns.tags.comp != Condition::NONE) {
      msg.append(" tags");
      msg.append(Condition::to_string(patterns.tags.comp, false));
      msg += '[';
      for(auto& p : patterns.tags) {
        msg.append(Condition::to_string(p.comp, true));
        msg.append("'" + p + "', ");
      }
      msg += ']';
    }
    error_msg(err, msg);
  }
  return schemas;
}

void Reader::read(std::string& buf, const char* stop, bool keep_escape) {
  uint32_t escape = 0;
  bool quote_1 = false;
  bool quote_2 = false;
  bool is_quoted = false;
  bool was_quoted = false;

  while(remain && !err) {
    if(!escape && *ptr == '\\') {
      escape = remain-1;
      if(!keep_escape) {
        ++ptr;
        --remain;
        continue;
      }
    } else if(escape && escape != remain)
      escape = 0;

    if(!escape) {
      if(!is_quoted && buf.empty() && found_space())
        continue;
      if(((!is_quoted || quote_1) && found_quote_single(quote_1)) ||
          ((!is_quoted || quote_2) && found_quote_double(quote_2))) {
        is_quoted = quote_1 || quote_2;
        was_quoted = true;
        continue;
      }
      if((was_quoted && !is_quoted) ||
          (!was_quoted && (found_space() || is_char(stop))))
        break;
    }
    buf += *ptr;
    ++ptr;
    --remain;
  }
}


void Reader::read_uint8_t(uint8_t& value, bool& was_set) {
  int64_t v;
  read_int64_t(v, was_set);
  if (!was_set || v > UINT8_MAX || v < INT8_MIN)
    error_msg(Error::SQL_PARSE_ERROR, " unsigned 8-bit integer out of range");
  else
    value = v;
}

void Reader::read_uint16_t(uint16_t& value, bool& was_set) {
  int64_t v;
  read_int64_t(v, was_set);
  if (!was_set || v > UINT16_MAX || v < INT16_MIN)
    error_msg(Error::SQL_PARSE_ERROR, " unsigned 16-bit integer out of range");
  else
    value = v;
}


void Reader::read_uint32_t(uint32_t& value, bool& was_set, const char* stop) {
  int64_t v;
  read_int64_t(v, was_set, stop);
  if (!was_set || v > UINT32_MAX || v < INT32_MIN)
    error_msg(Error::SQL_PARSE_ERROR, " unsigned 32-bit integer out of range");
  else
    value = v;
}

void Reader::read_int64_t(int64_t& value, bool& was_set, const char* stop) {
  std::string buf;
  read(buf, stop ? stop : "),]");
  if(err)
    return;
  try {
    value = std::stoll(buf);
    was_set = true;
  } catch(...) {
    error_msg(Error::SQL_PARSE_ERROR, " signed 64-bit integer out of range");
    was_set = false;
  }
}

void Reader::read_uint64_t(uint64_t& value, bool& was_set, const char* stop) {
  std::string buf;
  read(buf, stop ? stop : "),]");
  if(err)
    return;
  try {
    value = std::stoull(buf);
    was_set = true;
  } catch(...) {
    error_msg(
      Error::SQL_PARSE_ERROR, " unsigned 64-bit integer out of range");
    was_set = false;
  }
}

void Reader::read_double_t(long double& value, bool& was_set,
                           const char* stop) {
  std::string buf;
  read(buf, stop ? stop : "),]");
  if(err)
    return;
  try {
    value = std::stold(buf);
    was_set = true;
  } catch(...) {
    error_msg(Error::SQL_PARSE_ERROR, "double out of range");
    was_set = false;
  }
}

void Reader::read_duration_secs(uint64_t& value, bool& was_set, const char* stop) {
  std::string buf;
  read(buf, stop ? stop : "),]");
  try {
    size_t pos = 0;
    value = std::stoull(buf.c_str(), &pos);
    was_set = true;
    if(pos == buf.size())
      return;
    const char* p = buf.c_str() + pos;
    if(      *p == 'w' || Condition::str_case_eq(p, "week", 4))
      value *= 60 * 60 * 24 * 7;
    else if (*p == 'd' || Condition::str_case_eq(p, "day", 3))
      value *= 60 * 60 * 24;
    else if (*p == 'h' || Condition::str_case_eq(p, "hour", 4))
      value *= 60 * 60;
    else if (*p == 'm' ||Condition::str_case_eq(p, "minute", 6))
      value *= 60;
    else
      error_msg(Error::SQL_PARSE_ERROR, " unknown duration: "+std::string(p));
  } catch(...) {
    error_msg(Error::SQL_PARSE_ERROR, " signed 64-bit integer out of range");
    was_set = false;
  }
}

DB::Types::Encoder Reader::read_encoder() {
  std::string encoder;
  read(encoder, ")");
  DB::Types::Encoder enc = Core::Encoder::encoding_from(encoder);
  if(err || enc == DB::Types::Encoder::UNKNOWN)
    error_msg(Error::SQL_PARSE_ERROR, "bad encoder");
  return enc;
}

DB::Cell::Serial::Value::Type Reader::read_serial_value_type() {
  DB::Cell::Serial::Value::Type typ;
  if(found_token("INT64", 5) || found_token("I", 1)) {
    typ = DB::Cell::Serial::Value::Type::INT64;
  } else if (found_token("DOUBLE", 6) || found_token("D", 1)) {
    typ = DB::Cell::Serial::Value::Type::DOUBLE;
  } else if (found_token("BYTES", 5) || found_token("B", 1)) {
    typ = DB::Cell::Serial::Value::Type::BYTES;
  } else if (found_token("KEY", 3) || found_token("K", 1)) {
    typ = DB::Cell::Serial::Value::Type::KEY;
  } else if (found_token("LIST_INT64", 10) || found_token("LI", 2)) {
    typ = DB::Cell::Serial::Value::Type::LIST_INT64;
  } else if (found_token("LIST_BYTES", 10) || found_token("LB", 2)) {
    typ = DB::Cell::Serial::Value::Type::LIST_BYTES;
  } else {
    error_msg(
      Error::SQL_PARSE_ERROR, "Not Supported Serial Value Type");
    return DB::Cell::Serial::Value::Type::UNKNOWN;
  }
  seek_space();
  bool was_set;
  expect_token(":", 1, was_set);
  return typ;
}

void Reader::read_key(DB::Cell::Key& key) {
  bool bracket_square = false;
  std::string fraction;

  seek_space();
  expect_token("[", 1, bracket_square);

  while(remain && !err) {
    if(found_space())
      continue;

    read(fraction, ",]");
    key.add(fraction);
    fraction.clear();

    seek_space();
    if(found_char(','))
      continue;
    break;
  }

  expect_token("]", 1, bracket_square);
}

bool Reader::is_numeric_comparator(Condition::Comp& comp, bool _double) {
  switch(comp) {
    case Condition::NONE: {
      comp = Condition::EQ;
      return true;
    }
    case Condition::RE:
    case Condition::PF:
    case Condition::POSBS:
    case Condition::POSPS: {
      error_msg(Error::SQL_PARSE_ERROR,
        "unsupported numeric 'comparator' PF,RE,POSBS,POSPS");
      return false;
    }
    case Condition::FOSBS:
    case Condition::FOSPS: {
      if(_double) {
        error_msg(Error::SQL_PARSE_ERROR,
          "unsupported double numeric 'comparator' FOSPS,FOSBS");
        return false;
      }
      return true;
    }
    default:
      return true;
  }
}

void Reader::read_column_tags(DB::Schemas::TagsPattern& tags) {
  Condition::Comp comp = Condition::NONE;
  seek_space();
  found_comparator(comp, false);
  if(comp != Condition::NONE &&
     comp != Condition::NE &&
     comp != Condition::GT &&
     comp != Condition::LT &&
     comp != Condition::GE &&
     comp != Condition::LE &&
     comp != Condition::EQ &&
     comp != Condition::SBS &&
     comp != Condition::SPS &&
     comp != Condition::POSBS &&
     comp != Condition::FOSBS &&
     comp != Condition::POSPS &&
     comp != Condition::FOSPS ) {
    error_msg(Error::SQL_PARSE_ERROR,
      std::string("unsupported 'comparator' ") +
      Condition::to_string(comp, true)
    );
    return;
  }
  tags.comp = comp;
  seek_space();
  bool chk;
  expect_token("[", 1, chk);

  std::string buff;
  while(remain && !err) {
    if(found_char(',') || found_space())
      continue;
    if(found_char(']')) {
      if(tags.comp == Condition::NONE && !tags.empty()) {
        tags.comp = Condition::EQ;
      }
      return;
    }
    found_comparator(comp = Condition::NONE, true);
    seek_space();
    read(buff, ",]", comp == Condition::RE);
    if(!buff.empty() || comp != Condition::NONE) {
      tags.emplace_back(
        comp == Condition::NONE ? Condition::EQ : comp,
        std::move(buff)
      );
    }
  }
  expect_token("]", 1, chk);
}

void Reader::read_column(const char* stop,
                         std::string& col_name,
                         DB::Schemas::NamePatterns& names) {
  Condition::Comp comp = Condition::NONE;
  found_comparator(comp, true);
  read(col_name, stop, comp == Condition::RE);
  if(comp != Condition::NONE && comp != Condition::EQ) {
    if(col_name.empty()) {
      error_msg(
        Error::SQL_PARSE_ERROR,
        "expected column name(expression) after comparator"
      );
    } else {
      names.emplace_back(comp, std::move(col_name));
    }
  }
}


void Reader::read_ts_and_value(DB::Types::Column col_type, bool require_ts,
                               DB::Cells::Cell& cell) {
  std::string buf;
  read(buf, ",)");
  if(err)
    return;
  if(!buf.empty()) {
    if(Condition::str_case_eq(buf.c_str(), "auto", 4)) {
      cell.set_timestamp_auto();
    } else {
      cell.set_timestamp(Time::parse_ns(err, buf));
    }
  }
  if(err || (require_ts && buf.empty())) {
    error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
    return;
  }
  buf.clear();

  seek_space();
  if(err)
    return;

  switch(col_type) {
    case DB::Types::Column::PLAIN: {
      if(!found_char(','))
        break;
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
      if(!found_char(','))
        break;
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
      found_char(',');
      std::string value;
      read(value, ",)");
      if(err)
        return;
      const uint8_t* valp = reinterpret_cast<const uint8_t*>(value.c_str());
      size_t _remain = value.length();
      uint8_t op;
      int64_t v;
      counter_op_from(&valp, &_remain, op, v);
      if(err) {
        error_msg(Error::SQL_PARSE_ERROR, Error::get_text(err));
        return;
      }
      cell.set_counter(
        op,
        v,
        op & DB::Cells::OP_EQUAL
          ? col_type
          : DB::Types::Column::COUNTER_I64
      );
      break;
    }
    default:
      break;
  }
}

void Reader::counter_op_from(const uint8_t** ptrp, size_t* remainp,
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

void Reader::error_msg(int error, const std::string& msg) {
  err = error;
  auto at = sql.length() - remain + 1;
  message.append("error=");
  message.append(std::to_string(err));
  message.append("(");
  message.append(Error::get_text(err));
  message.append(")\n");

  message.append(" SQL='");
  message.append(sql);
  message.append("'\n");
  message.insert(message.length(), at + 5, ' ');
  message.append("^ at=");
  message.append(std::to_string(at));
  message.append("\n");
  message.insert(message.cend(), at + 5, ' ');
  message.append(msg);
  message.append("\n");
}




}}} // SWC::client:SQL namespace
