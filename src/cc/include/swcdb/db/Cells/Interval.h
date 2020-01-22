/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Interval_h
#define swcdb_db_Cells_Interval_h

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC {  namespace DB { namespace Cells {

class Interval final {

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

  ~Interval(){ 
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
  
  void set_key_begin(const DB::Cell::Key& key) {
    key_begin.copy(key);
    was_set = true;
  }

  void set_key_end(const DB::Cell::Key& key) {
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
    
    if(!initiated || !is_in_begin(other.key_begin))
      set_key_begin(other.key_begin);
  
    if(!initiated || !is_in_end(other.key_end))
      set_key_end(other.key_end);

    if(!initiated 
       || (!other.ts_earliest.empty() 
            && other.ts_earliest.comp == Condition::NONE)
       || !ts_earliest.is_matching(other.ts_earliest.value))
      set_ts_earliest(other.ts_earliest);

    if(!initiated
       || (!other.ts_latest.empty() 
            && other.ts_latest.comp == Condition::NONE)
       || !ts_latest.is_matching(other.ts_latest.value))
      set_ts_latest(other.ts_latest);
  }

  void expand(const Cell& cell) {
    expand_begin(cell);
    expand_end(cell);

    expand(cell.timestamp);
    was_set = true;
  }

  void expand_begin(const Cell& cell) {
    if(key_begin.empty() || !is_in_begin(cell.key))
      key_begin.copy(cell.key);
    expand(cell.timestamp);
    was_set = true;
  }

  void expand_end(const Cell& cell) {
    if(key_end.empty() || !is_in_end(cell.key))
      key_end.copy(cell.key);
    expand(cell.timestamp);
    was_set = true;
  }

  void expand(const int64_t& ts) {
    if(ts_earliest.empty() || !ts_earliest.is_matching(ts))
      ts_earliest.set(ts, Condition::GE);
    if(ts_latest.empty() || !ts_latest.is_matching(ts))
      ts_latest.set(ts, Condition::LE);
    was_set = true;
  }

  const bool equal(const Interval& other) const {
    return
      was_set == other.was_set &&
      key_begin.equal(other.key_begin) && 
      key_end.equal(other.key_end) && 
      ts_earliest.equal(other.ts_earliest) && 
      ts_latest.equal(other.ts_latest);
  }

  const bool is_in_begin(const DB::Cell::Key &key) const {
    return key_begin.empty() || 
          (!key.empty() && key_begin.compare(key) != Condition::LT);
  }
  
  const bool is_in_end(const DB::Cell::Key &key) const {
    return key_end.empty() || 
          (!key.empty() && key_end.compare(key) != Condition::GT);
  }

  const bool consist(const Interval& other) const {
    return (other.key_end.empty()   || is_in_begin(other.key_end)) 
        && (other.key_begin.empty() || is_in_end(other.key_begin));
  }

  const bool consist(const DB::Cell::Key& key) const {
    return is_in_begin(key) && is_in_end(key);
  }

  const bool consist(const DB::Cell::Key& key, int64_t ts) const {
    return is_in_begin(key) 
          && 
           is_in_end(key) 
          &&
           (ts_earliest.empty() || ts_earliest.is_matching(ts))
          &&    
           (ts_latest.empty() || ts_latest.is_matching(ts));
  }

  const bool includes(const Interval& other) const {
    return other.key_begin.empty() || other.key_end.empty() ||
           consist(other);           
  }

  const bool includes(const Specs::Interval::Ptr interval) const {
    return includes(*interval.get());
  }

  const bool includes(const Specs::Interval& interval) const { 
    return  (key_end.empty() || interval.is_matching_begin(key_end))
            && 
            (key_begin.empty() || interval.is_matching_end(key_begin))
          ;
      /* // , bool ts=false
      && 
      (!ts || (
        (ts_latest.empty() || 
         interval.ts_start.is_matching(ts_latest.value) )
        && 
        (ts_earliest.empty() || 
         interval.ts_finish.is_matching(ts_earliest.value) )
      ) )
      */
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


  DB::Cell::Key     key_begin;
  DB::Cell::Key     key_end;  
  Specs::Timestamp  ts_earliest;
  Specs::Timestamp  ts_latest;
  bool              was_set = false;
};

}}}
#endif