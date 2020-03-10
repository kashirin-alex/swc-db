/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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

const bool Timestamp::empty() const {
  return !was_set;
}

const bool Timestamp::equal(const Timestamp &other) const {
  return value == other.value && comp == other.comp;
}

const size_t Timestamp::encoded_length() const {
  return 1+(comp != Condition::NONE? 8: 0);
}

void Timestamp::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Condition::NONE)
    Serialization::encode_i64(bufp, value);
}

void Timestamp::decode(const uint8_t **bufp, size_t *remainp){
  comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
  if(comp != Condition::NONE)
    value = Serialization::decode_i64(bufp, remainp);
}

const bool Timestamp::is_matching(int64_t other) const {
  return Condition::is_matching(comp, value, other);
}

const std::string Timestamp::to_string() const {
  std::string s("Timestamp(");
  s.append(Condition::to_string(comp));
  if(comp != Condition::NONE)
    s.append(std::to_string(value));
  s.append(")");
  return s;
}

void Timestamp::display(std::ostream& out) const {
  out << Condition::to_string(comp) << " \"" << value << "\"";
}



}}}
