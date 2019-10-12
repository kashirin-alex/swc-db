/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Interval_h
#define swcdb_db_Cells_Interval_h

#include "swcdb/lib/db/Cells/Cell.h"
#include "SpecsInterval.h"


namespace SWC {  namespace DB { namespace Cells {

class Interval {

  /* encoded-format: 
      key_begin-encoded key_end-encoded vi64(ts_earliest) vi64(ts_latest)
  */

  public:

  explicit Interval() { }

  explicit Interval(const uint8_t **ptr, size_t *remain) {
    decode(ptr, remain, true); 
  }

  explicit Interval(const Interval& other) {
    copy(other); 
  }

  virtual ~Interval(){ 
    free();
  }

  void copy(const Interval& other) {
    set_key_begin(other.key_begin);
    set_key_end(other.key_end);

    set_ts_earliest(other.ts_earliest);
    set_ts_latest(other.ts_latest);
    was_set = true;
  }

  void free() {
    key_begin.free();
    key_end.free();
    ts_earliest.free();
    ts_latest.free();
    was_set = false;
  }
  
  void set_key_begin(const Specs::Key& key) {
    key_begin.copy(key);
    was_set = true;
  }

  void set_key_end(const Specs::Key& key) {
    key_end.copy(key);
    was_set = true;
  }

  void set_ts_earliest(const Specs::Timestamp& ts) {
    ts_earliest.copy(ts);
    was_set = true;
  }

  void set_ts_latest(const Specs::Timestamp& ts) {
    ts_latest.copy(ts);
    was_set = true;
  }

  void expand(const Interval& other) {
    bool initiated = was_set;
    
    if(!initiated || !key_begin.is_matching(other.key_begin))
      set_key_begin(other.key_begin);
  
    if(!initiated || !key_end.is_matching(other.key_end))
      set_key_end(other.key_end);

    if(!initiated || !ts_earliest.is_matching(other.ts_earliest.value))
      set_ts_earliest(other.ts_earliest);
    if(!initiated || !ts_latest.is_matching(other.ts_latest.value))
      set_ts_latest(other.ts_latest);
  }

  void expand(const Cell& cell) {
    if(key_begin.empty() || !key_begin.is_matching(cell.key))
      key_begin.set(cell.key, Condition::GE);
    if(key_end.empty() || !key_end.is_matching(cell.key))
      key_end.set(cell.key, Condition::LE);
    
    if(ts_earliest.empty() || !ts_earliest.is_matching(cell.timestamp))
      ts_earliest.set(cell.timestamp, Condition::GE);
    if(ts_latest.empty() || !ts_latest.is_matching(cell.timestamp))
      ts_latest.set(cell.timestamp, Condition::LE);
    was_set = true;
  }

  const bool equal(const Interval& other) const {
    return
      key_begin.equal(other.key_begin) && 
      key_end.equal(other.key_end) && 
      ts_earliest.equal(other.ts_earliest) && 
      ts_latest.equal(other.ts_latest);
  }

  const bool is_in_begin(const Specs::Key &key) const {
    return key_begin.is_matching(key);
  }
  
  const bool is_in_end(const Specs::Key &key) const {
    return key_end.is_matching(key);
  }

  const bool consist(const Interval& other) const {
    return key_begin.is_matching(other.key_end) && 
           key_end.is_matching(other.key_begin);
  }

  const bool includes(const Interval& other) const {
    return key_begin.empty() || key_end.empty() ||
           other.key_begin.empty() || other.key_end.empty() ||
           (
            key_begin.is_matching(other.key_end) 
            &&
            key_end.is_matching(other.key_begin)
           );           
  }

  const bool includes(const Specs::Interval::Ptr interval) const {
    return key_begin.empty() || key_end.empty() ||
           interval->key_start.empty() || interval->key_finish.empty() ||
           (
            key_begin.is_matching(interval->key_finish) 
            &&
            key_end.is_matching(interval->key_start)
           );
           
  }

  const bool consist(const DB::Cell::Key& key) const {
    return key_begin.is_matching(key) && key_end.is_matching(key);
  }

  const size_t encoded_length() const {
    return  key_begin.encoded_length()
          + key_end.encoded_length()
          + ts_earliest.encoded_length()
          + ts_latest.encoded_length();
  }

  void encode(uint8_t **ptr) const {
    key_begin.encode(ptr);
    key_end.encode(ptr);
    ts_earliest.encode(ptr);
    ts_latest.encode(ptr);
  }

  void decode(const uint8_t **ptr, size_t *remain, bool owner=false){
    key_begin.decode(ptr, remain, owner);
    key_end.decode(ptr, remain, owner);
    ts_earliest.decode(ptr, remain);
    ts_latest.decode(ptr, remain);
    was_set = true;
  }

  const std::string to_string() const {
    std::string s("Interval(begin=");
    s.append(key_begin.to_string());
    s.append( " end=");
    s.append(key_end.to_string());
    s.append( " earliest=");
    s.append(ts_earliest.to_string());
    s.append( " latest=");
    s.append(ts_latest.to_string());
    s.append( " was_set=");
    s.append(was_set?"true":"false");
    
    return s;
  }


  Specs::Key        key_begin;
  Specs::Key        key_end;  
  Specs::Timestamp  ts_earliest;
  Specs::Timestamp  ts_latest;
  bool              was_set = false;
};

}}}
#endif