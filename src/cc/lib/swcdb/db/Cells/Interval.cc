/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Interval.h"



namespace SWC {  namespace DB { namespace Cells {


Interval::Interval(const Types::KeySeq key_seq) 
                  : key_seq(key_seq) { 
}

Interval::Interval(const Types::KeySeq key_seq,
                   const uint8_t **ptr, size_t *remain) 
                  : key_seq(key_seq) {
  decode(ptr, remain, true); 
}

Interval::Interval(const Interval& other) 
                  : key_seq(other.key_seq) {
  copy(other); 
}

Interval::~Interval(){ 
  free();
}

void Interval::copy(const Interval& other) {
  set_key_begin(other.key_begin);
  set_key_end(other.key_end);

  set_ts_earliest(other.ts_earliest);
  set_ts_latest(other.ts_latest);

  set_aligned_min(other.aligned_min);
  set_aligned_max(other.aligned_max);
  was_set = true;
}

void Interval::free() {
  key_begin.free();
  key_end.free();
  ts_earliest.free();
  ts_latest.free();
  was_set = false;
  aligned_min.free();
  aligned_max.free();
}

size_t Interval::size_of_internal() const {
  return key_begin.size + key_end.size + 
         aligned_min.size_of_internal() + 
         aligned_min.size_of_internal();
}

void Interval::set_key_begin(const DB::Cell::Key& key) {
  key_begin.copy(key);
  was_set = true;
}

void Interval::set_key_end(const DB::Cell::Key& key) {
  key_end.copy(key);
  was_set = true;
}

void Interval::set_ts_earliest(const Specs::Timestamp& ts) {
  ts_earliest.copy(ts);
  was_set = true;
}

void Interval::set_ts_latest(const Specs::Timestamp& ts) {
  ts_latest.copy(ts);
  was_set = true;
}

void Interval::set_aligned_min(const DB::Cell::KeyVec& key) {
  aligned_min.copy(key);
  was_set = true;
}

void Interval::set_aligned_max(const DB::Cell::KeyVec& key) {
  aligned_max.copy(key);
  was_set = true;
}

void Interval::expand(const Interval& other) {
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

void Interval::expand(const Cell& cell) {
  expand_begin(cell);
  expand_end(cell);
  //expand(cell.timestamp);
  was_set = true;
}

void Interval::expand_begin(const Cell& cell) {
  if(key_begin.empty() || !is_in_begin(cell.key))
    key_begin.copy(cell.key);
  was_set = true;
}

void Interval::expand_end(const Cell& cell) {
  if(key_end.empty() || !is_in_end(cell.key))
    key_end.copy(cell.key);
  was_set = true;
}

void Interval::expand(const int64_t& ts) {
  if(ts_earliest.empty() || !ts_earliest.is_matching(ts))
    ts_earliest.set(ts, Condition::GE);
  if(ts_latest.empty() || !ts_latest.is_matching(ts))
    ts_latest.set(ts, Condition::LE);
  was_set = true;
}

bool Interval::align(const Interval &other) {
  bool start = DB::KeySeq::align(
    key_seq, aligned_min, other.aligned_min, Condition::LT);
  bool finish = DB::KeySeq::align(
    key_seq, aligned_max, other.aligned_max, Condition::GT);
  return start || finish;
}

bool Interval::align(const DB::Cell::Key &key) {
  return DB::KeySeq::align(key_seq, key, aligned_min, aligned_max);
}

bool Interval::equal(const Interval& other) const {
  return
      was_set == other.was_set &&

    key_begin.equal(other.key_begin) && 
    key_end.equal(other.key_end) && 

    ts_earliest.equal(other.ts_earliest) && 
    ts_latest.equal(other.ts_latest) &&

    aligned_min.equal(other.aligned_min) && 
    aligned_max.equal(other.aligned_max);
}

bool Interval::is_in_begin(const DB::Cell::Key &key) const {
  return key_begin.empty() || 
        (!key.empty() && 
          DB::KeySeq::compare(key_seq, key_begin, key) != Condition::LT);
}

bool Interval::is_in_end(const DB::Cell::Key &key) const {
  return key_end.empty() || 
        (!key.empty() && 
          DB::KeySeq::compare(key_seq, key_end, key) != Condition::GT);
}

bool Interval::consist(const Interval& other) const {
  return (other.key_end.empty()   || is_in_begin(other.key_end)) 
      && (other.key_begin.empty() || is_in_end(other.key_begin));
}

bool Interval::consist(const DB::Cell::Key& key) const {
  return is_in_begin(key) && is_in_end(key);
}

bool Interval::consist(const DB::Cell::Key& key, int64_t ts) const {
  return is_in_begin(key) 
        && 
         is_in_end(key) 
        &&
         (ts_earliest.empty() || ts_earliest.is_matching(ts))
        &&    
         (ts_latest.empty() || ts_latest.is_matching(ts));
}

bool Interval::includes(const Interval& other) const {
  return other.key_begin.empty() || other.key_end.empty() ||
         consist(other);           
}

bool Interval::includes_begin(const Specs::Interval& interval) const {
  return key_begin.empty() || interval.is_matching_end(key_seq, key_begin);
}

bool Interval::includes_end(const Specs::Interval& interval) const {
  return key_end.empty() || interval.is_matching_begin(key_seq, key_end);
}

bool Interval::includes(const Specs::Interval& interval) const {
  return  includes_begin(interval) && includes_end(interval);
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

size_t Interval::encoded_length() const {
  return  key_begin.encoded_length()
        + key_end.encoded_length()
        + ts_earliest.encoded_length()
        + ts_latest.encoded_length()
        + aligned_min.encoded_length()
        + aligned_max.encoded_length();
}

void Interval::encode(uint8_t **ptr) const {
  key_begin.encode(ptr);
  key_end.encode(ptr);
  ts_earliest.encode(ptr);
  ts_latest.encode(ptr);
  aligned_min.encode(ptr);
  aligned_max.encode(ptr);
}

void Interval::decode(const uint8_t **ptr, size_t *remain, bool owner){
  key_begin.decode(ptr, remain, owner);
  key_end.decode(ptr, remain, owner);
  ts_earliest.decode(ptr, remain);
  ts_latest.decode(ptr, remain);
  aligned_min.decode(ptr, remain);
  aligned_max.decode(ptr, remain);
  
  was_set = true;
}

std::string Interval::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Interval::print(std::ostream& out) const {
  out 
    << "Interval("
    << "begin="     << key_begin
    << " end="      << key_end
    << " earliest=" << ts_earliest
    << " latest="   << ts_latest
    << " min="      << aligned_min
    << " max="      << aligned_max
    << " was_set=" << (was_set? "true" : "false")
    << ')';
}


}}}
