/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsValue.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC { namespace DB { namespace Specs {

Value::Value(bool own)
              : own(own), comp(Condition::NONE),
                data(0), size(0), matcher(nullptr) {
}

Value::Value(const char* data_n, Condition::Comp comp_n, bool owner)
            : own(false), matcher(nullptr) {
  set((uint8_t*)data_n, strlen(data_n), comp_n, owner);
}

Value::Value(const char* data_n, const uint32_t size_n,
             Condition::Comp comp_n, bool owner)
            : own(false), matcher(nullptr) {
  set((uint8_t*)data_n, size_n, comp_n, owner);
}

Value::Value(const uint8_t* data_n, const uint32_t size_n,
             Condition::Comp comp_n, bool owner)
            : own(false), matcher(nullptr) {
  set(data_n, size_n, comp_n, owner);
}

Value::Value(int64_t count, Condition::Comp comp_n)
            : own(false), matcher(nullptr) {
  set_counter(count, comp_n);
}

Value::Value(const Value &other)
            : own(false), matcher(nullptr) {
  copy(other);
}

Value::Value(Value&& other)
            : own(other.own), comp(other.comp),
              data(other.data), size(other.size),
              matcher(other.matcher) {
  other.comp = Condition::NONE;
  other.data = nullptr;
  other.matcher = nullptr;
  other.size = 0;
}

void Value::set_counter(int64_t count, Condition::Comp comp_n) {
  uint32_t len = Serialization::encoded_length_vi64(count);
  uint8_t data_n[10];
  uint8_t* ptr = data_n;
  Serialization::encode_vi64(&ptr, count);
  set(data_n, len, comp_n, true);
}

void Value::set(const char* data_n, Condition::Comp comp_n, bool owner) {
  set((uint8_t*)data_n, strlen(data_n), comp_n, owner);
}

void Value::set(const std::string& data_n, Condition::Comp comp_n) {
  set((uint8_t*)data_n.data(), data_n.length(), comp_n, true);
}

void Value::copy(const Value &other) {
  set(other.data, other.size, other.comp, true);
}

void Value::set(const uint8_t* data_n, const uint32_t size_n,
                Condition::Comp comp_n, bool owner) {
  free();
  own   = owner;
  comp = comp_n;
  if((size = size_n))
    data = own ? (uint8_t*)memcpy(new uint8_t[size], data_n, size)
               : (uint8_t*)data_n;
}

Value::~Value() {
  _free();
}

void Value::_free() {
  if(own && data)
    delete [] data;

  if(matcher)
    delete matcher;
}

void Value::free() {
  _free();
  data = nullptr;
  size = 0;
  matcher = nullptr;
}

SWC_SHOULD_INLINE
bool Value::empty() const {
  return comp == Condition::NONE;
}

bool Value::equal(const Value &other) const {
  return size == other.size &&
         ((!data && !other.data) ||
          (data && other.data && !memcmp(data, other.data, size)));
}

size_t Value::encoded_length() const {
  return 1 + (comp == Condition::NONE
    ? 0 : Serialization::encoded_length_vi32(size) + size);
}

void Value::encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Condition::NONE) {
    Serialization::encode_vi32(bufp, size);
    memcpy(*bufp, data, size);
    *bufp+=size;
  }
}

void Value::decode(const uint8_t** bufp, size_t* remainp) {
  own = false;
  comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
  if(comp != Condition::NONE) {
    size = Serialization::decode_vi32(bufp, remainp);
    data = (uint8_t*)*bufp;
    *remainp -= size;
    *bufp += size;
  }
}

bool Value::is_matching(Types::Column col_type,
                        const Cells::Cell& cell) const {
  switch(col_type) {
    case Types::Column::PLAIN:
      return is_matching_plain(cell);
    case Types::Column::SERIAL:
      return is_matching_serial(cell);
    case Types::Column::COUNTER_I64:
    case Types::Column::COUNTER_I32:
    case Types::Column::COUNTER_I16:
    case Types::Column::COUNTER_I8:
      return is_matching_counter(cell);
    default:
      return false;
  }
}

struct MatcherPlainRE : Value::TypeMatcher {
  MatcherPlainRE(const uint8_t* data, uint32_t size)
                : re(re2::StringPiece((const char*)data, size)) {
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
      if(!matcher)
        matcher = new MatcherPlainRE(data, size);
      return Condition::re(
        ((MatcherPlainRE*)matcher)->re, (const char*)v.base, v.size);
    }
    default:
      return Condition::is_matching_extended(comp, data, size, v.base, v.size);
  }
}

struct MatcherSerial : Value::TypeMatcher {
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
  return ((MatcherSerial*)matcher)->fields.is_matching(cell)
    ? comp == Condition::EQ
    : comp == Condition::NE;
}

struct MatcherCounter : Value::TypeMatcher {
  MatcherCounter(const uint8_t* data, uint32_t size) {
    errno = 0;
    char *last = (char*)data + size;
    value = strtoll((const char*)data, &last, 0);
  }
  int64_t value;
};

bool Value::is_matching_counter(const Cells::Cell& cell) const {
  if(empty())
    return true;
  if(!matcher)
    matcher = new MatcherCounter(data, size);
  return Condition::is_matching(
    comp, ((MatcherCounter*)matcher)->value, cell.get_counter());
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
