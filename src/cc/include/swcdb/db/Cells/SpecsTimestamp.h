/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsTimestamp_h
#define swcdb_db_cells_SpecsTimestamp_h

#include "swcdb/core/Serializable.h"
#include "swcdb/core/Comparators.h"


namespace SWC { namespace DB { namespace Specs {

     
class Timestamp {
  public:

  explicit Timestamp(): value(0), comp(Condition::NONE), was_set(false) {}

  explicit Timestamp(int64_t timestamp, Condition::Comp comp) {
    set(timestamp, comp);
  }
  
  explicit Timestamp(const Timestamp &other){
    copy(other);
  }

  void copy(const Timestamp &other) {
    //std::cout << " copy(const Timestamp &other)\n";
    set(other.value, other.comp);
  }

  void set(int64_t timestamp, Condition::Comp comperator) {
    value = timestamp;
    comp  = comperator;
    was_set = true;
  }

  void free() {
    value  = 0;
    comp  = Condition::NONE;
    was_set = false;
  }

  virtual ~Timestamp(){
    //std::cout << " ~Timestamp\n";
  }

  const bool empty() const {
    return !was_set;
  }

  const bool equal(const Timestamp &other) const {
    return value == other.value && comp == other.comp;
  }

  const size_t encoded_length() const {
    return 1+(comp != Condition::NONE? 8: 0);
  }

  void encode(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)comp);
    if(comp != Condition::NONE)
      Serialization::encode_i64(bufp, value);
  }

  void decode(const uint8_t **bufp, size_t *remainp){
    comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
    if(comp != Condition::NONE)
      value = Serialization::decode_i64(bufp, remainp);
  }

  const bool is_matching(int64_t other) const {
    return Condition::is_matching(comp, value, other);
  }

  const std::string to_string() const {
    std::string s("Timestamp(");
    s.append(Condition::to_string(comp));
    if(comp != Condition::NONE)
      s.append(std::to_string(value));
    s.append(")");
    return s;
  }

  void display(std::ostream& out) const {
    out << Condition::to_string(comp) << " \"" << value << "\"";
  }

  int64_t          value; 
  Condition::Comp  comp;
  bool             was_set;
};

}}}

#endif