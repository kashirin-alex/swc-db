/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsInterval_h
#define swcdb_db_cells_SpecsInterval_h


#include "swcdb/lib/core/Serializable.h"

#include "SpecsKey.h"
#include "SpecsValue.h"
#include "SpecsTimestamp.h"
#include "SpecsFlags.h"


namespace SWC { namespace DB { namespace Specs {

class Interval {
  public:

  typedef std::shared_ptr<Interval> Ptr;

  inline static Ptr make_ptr(){
    return std::make_shared<Interval>();
  }

  inline static Ptr make_ptr(
      const Key& key_start, const Key& key_finish, const Value& value, 
      const Timestamp& ts_start, const Timestamp& ts_finish, 
      const Flags& flags=Flags()) {
    return std::make_shared<Interval>(
      key_start, key_finish, value, ts_start, ts_finish, flags);
  }
  
  inline static Ptr make_ptr(const uint8_t **bufp, size_t *remainp){
    return std::make_shared<Interval>(bufp, remainp);
  }

  inline static Ptr make_ptr(const Interval& other){
    return std::make_shared<Interval>(other);
  }

  inline static Ptr make_ptr(Ptr other){
    return std::make_shared<Interval>(*other.get());
  }
  
  explicit Interval() {}

  explicit Interval(const Key& key_start, const Key& key_finish, const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags())
                    : key_start(key_start), key_finish(key_finish), value(value),
                      ts_start(ts_start), ts_finish(ts_finish), flags(flags) {}
  
  explicit Interval(const uint8_t **bufp, size_t *remainp) {
    decode(bufp, remainp); 
  }

  explicit Interval(const Interval& other) {
    copy(other);
  }

  void copy(const Interval& other) {
    //std::cout  << " copy(const Interval& other)\n";

    key_start.copy(other.key_start);
    key_finish.copy(other.key_finish);

    value.copy(other.value);

    ts_start.copy(other.ts_start);
    ts_finish.copy(other.ts_finish);

    flags.copy(other.flags);
  }

  virtual ~Interval(){
    //std::cout << " ~Interval\n";
    free();
  }
  
  void free() {
    key_start.free();
    key_finish.free();
    value.free();
  }

  void expand(const Cells::Cell& cell) {
    if(key_start.empty() || !key_start.is_matching(cell.key)){
      key_start.set(cell.key, Condition::GE);
    }
    if(key_finish.empty() || !key_finish.is_matching(cell.key)){
      key_finish.set(cell.key, Condition::LE);
    }
  }

  bool equal(const Interval& other) const {
    return  ts_start.equal(other.ts_start) &&
            ts_finish.equal(other.ts_finish) &&
            flags.equal(other.flags) &&
            key_start.equal(other.key_start) &&
            key_finish.equal(other.key_finish) &&
            value.equal(other.value) ;
  }

  const bool is_matching(const Cells::Cell& cell, 
                         Types::Column typ=Types::Column::PLAIN) const {
    bool match = 
      ts_start.is_matching(cell.timestamp) &&
      ts_finish.is_matching(cell.timestamp) &&
      (key_start.empty()  || key_start.is_matching(cell.key)) &&
      (key_finish.empty() || key_finish.is_matching(cell.key));
    if(!match)
      return match;
    
    switch(typ) {
      case Types::Column::COUNTER_I64: 
        return value.is_matching(cell.get_value());
      default:
        return value.is_matching(cell.value, cell.vlen);
    }
  }

  const size_t encoded_length() const {
    return key_start.encoded_length() + key_finish.encoded_length()
          + value.encoded_length()
          + ts_start.encoded_length() + ts_finish.encoded_length()
          + flags.encoded_length();
  }

  void encode(uint8_t **bufp) const {
    key_start.encode(bufp);
    key_finish.encode(bufp);
    value.encode(bufp);
    ts_start.encode(bufp);
    ts_finish.encode(bufp);
    flags.encode(bufp);
  }

  void decode(const uint8_t **bufp, size_t *remainp){
    key_start.decode(bufp, remainp);
    key_finish.decode(bufp, remainp);
    value.decode(bufp, remainp);
    ts_start.decode(bufp, remainp);
    ts_finish.decode(bufp, remainp);
    flags.decode(bufp, remainp);
  }
  
  const std::string to_string() const {
    std::string s("Interval(Start");
    s.append(key_start.to_string());
    s.append(" Finish");
    s.append(key_finish.to_string());
    s.append(" ");
    s.append(value.to_string());
    s.append(" Start");
    s.append(ts_start.to_string());
    s.append(" Finish");
    s.append(ts_finish.to_string());
    s.append(" ");
    s.append(flags.to_string());
    return s;
  }

  Key        key_start, key_finish;
  Value      value;
  Timestamp  ts_start, ts_finish;
  Flags      flags;
};


}}}
#endif