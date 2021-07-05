/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsValue.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC { namespace DB { namespace Specs {



struct MatcherPlainRE : Value::TypeMatcher {
  SWC_CAN_INLINE
  MatcherPlainRE(const uint8_t* data, uint32_t size)
    : re(re2::StringPiece(reinterpret_cast<const char*>(data), size)) {
  }
  re2::RE2 re;
};

bool Value::is_matching_plain(const Cells::Cell& cell) const {
  if(empty())
    return true;

  StaticBuffer v;
  cell.get_value(v);
  switch(comp) {
    case Condition::RE: {
      if(!data || !size)
        return !v.base || !v.size;
      if(!matcher)
        matcher = new MatcherPlainRE(data, size);
      return Condition::re(
        static_cast<MatcherPlainRE*>(matcher)->re,
        reinterpret_cast<char*>(v.base), v.size);
    }
    default:
      return Condition::is_matching_extended(comp, data, size, v.base, v.size);
  }
}


struct MatcherSerial : Value::TypeMatcher {
  SWC_CAN_INLINE
  MatcherSerial(const uint8_t* data, uint32_t size)
                : fields(data, size) {
  }
  Serial::Value::Fields fields;
};

bool Value::is_matching_serial(const Cells::Cell& cell) const {
  if(empty())
    return true;
  if(!matcher)
    matcher = new MatcherSerial(data, size);
  return static_cast<MatcherSerial*>(matcher)->fields.is_matching(cell)
    ? comp == Condition::EQ
    : comp == Condition::NE;
}


struct MatcherCounter : Value::TypeMatcher {
  SWC_CAN_INLINE
  MatcherCounter(const uint8_t* data, uint32_t size) {
    errno = 0;
    char *last = reinterpret_cast<char*>(const_cast<uint8_t*>(data)) + size;
    value = strtoll(reinterpret_cast<const char*>(data), &last, 0);
  }
  int64_t value;
};

bool Value::is_matching_counter(const Cells::Cell& cell) const {
  if(empty())
    return true;
  if(!matcher)
    matcher = new MatcherCounter(data, size);
  return Condition::is_matching(
    comp, static_cast<MatcherCounter*>(matcher)->value, cell.get_counter());
}


std::string Value::to_string(Types::Column col_type) const {
  std::stringstream ss;
  print(col_type, ss);
  return ss.str();
}

void Value::print(Types::Column col_type, std::ostream& out) const {
  out << "Value(";
  if(size)
    display(col_type, out);
  out << ')';
}

void Value::display(Types::Column col_type, std::ostream& out,
                    bool pretty) const {
  out << "size=" << size << ' ' << Condition::to_string(comp, true);
  if(size) {
    if(col_type == Types::Column::SERIAL) {
      Serial::Value::Fields(data, size).print(out);
    } else {
      out << '"';
      char hex[5];
      hex[4] = '\0';
      const uint8_t* end = data + size;
      for(const uint8_t* ptr = data; ptr < end; ++ptr) {
        if(*ptr == '"')
          out << '\\';
        if(!pretty || (31 < *ptr && *ptr < 127)) {
          out << *ptr;
        } else {
          sprintf(hex, "0x%X", *ptr);
          out << hex;
        }
      }
      out << '"';
    }
  }
}



}}}
