/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/client/sql/SQL.h"


namespace SWC { namespace client { namespace SQL {


Reader::Reader(const std::string& sql, std::string& message)
              : sql(sql), message(message),
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

DB::Schema::Ptr Reader::get_schema(const std::string& col) {
  DB::Schema::Ptr schema;
  if(std::find_if(col.begin(), col.end(),
      [](unsigned char c){ return !std::isdigit(c); } ) != col.end()){
    schema = Env::Clients::get()->schemas->get(err, col);
  } else {
    try {
      schema = Env::Clients::get()->schemas->get(err, std::stoll(col));
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      err = e.code();
    }
  }
  if(err)
    error_msg(err, "problem getting column '"+col+"' schema");
  return schema;
}

std::vector<DB::Schema::Ptr>
Reader::get_schema(const std::vector<DB::Schemas::Pattern>& patterns) {
  auto schemas = Env::Clients::get()->schemas->get(err, patterns);
  if(err) {
    std::string msg("problem getting columns on patterns=[");
    for(auto& p : patterns) {
      msg.append(Condition::to_string(p.comp, true));
      msg.append("'" + p.value + "', ");
    }
    error_msg(err, msg + "] schema");
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
  if (v > UINT8_MAX || v < INT8_MIN)
    error_msg(Error::SQL_PARSE_ERROR, " unsigned 8-bit integer out of range");
  else
    value = v;
}

void Reader::read_uint16_t(uint16_t& value, bool& was_set) {
  int64_t v;
  read_int64_t(v, was_set);
  if (v > UINT16_MAX || v < INT16_MIN)
    error_msg(Error::SQL_PARSE_ERROR, " unsigned 16-bit integer out of range");
  else
    value = v;
}


void Reader::read_uint32_t(uint32_t& value, bool& was_set, const char* stop) {
  int64_t v;
  read_int64_t(v, was_set, stop);
  if (v > UINT32_MAX || v < INT32_MIN)
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
  message.insert(message.end(), at + 5, ' ');
  message.append(msg);
  message.append("\n");
}




}}} // SWC::client:SQL namespace
