/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsTimestamp.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Timestamp::Timestamp(): value(0), comp(Condition::NONE), was_set(false) {}

Timestamp::Timestamp(int64_t timestamp, Condition::Comp comp) {
  set(timestamp, comp);
}

Timestamp::Timestamp(const Timestamp &other){
  copy(other);
}

void Timestamp::copy(const Timestamp &other) {
  set(other.value, other.comp);
}

void Timestamp::set(int64_t timestamp, Condition::Comp comperator) {
  value = timestamp;
  comp  = comperator;
  was_set = true;
}

void Timestamp::free() {
  value  = 0;
  comp  = Condition::NONE;
  was_set = false;
}

Timestamp::~Timestamp() { }

bool Timestamp::empty() const {
  return !was_set;
}

bool Timestamp::equal(const Timestamp &other) const {
  return value == other.value && comp == other.comp;
}

size_t Timestamp::encoded_length() const {
  return 1 + (comp == Condition::NONE? 0: 8);
}

void Timestamp::encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Condition::NONE)
    Serialization::encode_i64(bufp, value);
}

void Timestamp::decode(const uint8_t** bufp, size_t* remainp){
  comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
  if(comp != Condition::NONE)
    value = Serialization::decode_i64(bufp, remainp);
}

bool Timestamp::is_matching(int64_t other) const {
  return Condition::is_matching(comp, value, other);
}

void Timestamp::display(std::ostream& out) const {
  out << Condition::to_string(comp) << " \"" << value << "\"";
}

std::string Timestamp::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Timestamp::print(std::ostream& out) const {
  out << "Timestamp(";
  if(comp != Condition::NONE)
    out << Condition::to_string(comp) << value;
  out << ')';
}



}}}
