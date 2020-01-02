/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsInterval_h
#define swcdb_db_cells_SpecsInterval_h


#include "swcdb/core/Serializable.h"

#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/db/Cells/SpecsValue.h"
#include "swcdb/db/Cells/SpecsTimestamp.h"
#include "swcdb/db/Cells/SpecsFlags.h"


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
  
  inline static Ptr make_ptr(
      const Cell::Key& range_begin, const Cell::Key& range_end,
      const Key& key_start, const Key& key_finish, 
      const Value& value, 
      const Timestamp& ts_start, const Timestamp& ts_finish, 
      const Flags& flags=Flags()) {
    return std::make_shared<Interval>(
      range_begin, range_end, 
      key_start, key_finish, value, ts_start, ts_finish, flags
    );
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
  
  explicit Interval() : offset_rev(0) {}

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end)
                    : range_begin(range_begin), range_end(range_end), 
                      offset_rev(0) {
  }

  explicit Interval(const Key& key_start, const Key& key_finish, 
                    const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags())
                    : key_start(key_start), key_finish(key_finish), 
                      value(value),
                      ts_start(ts_start), ts_finish(ts_finish), 
                      flags(flags), offset_rev(0) {
  }

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end, 
                    const Key& key_start, const Key& key_finish, 
                    const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags())
                    : range_begin(range_begin), range_end(range_end), 
                      key_start(key_start), key_finish(key_finish), 
                      value(value),
                      ts_start(ts_start), ts_finish(ts_finish), 
                      flags(flags), offset_rev(0) {
  }
  
  explicit Interval(const uint8_t **bufp, size_t *remainp) {
    decode(bufp, remainp); 
  }

  explicit Interval(const Interval& other) {
    copy(other);
  }

  void copy(const Interval& other) {
    //std::cout  << " copy(const Interval& other)\n";

    range_begin.copy(other.range_begin);
    range_end.copy(other.range_end);

    key_start.copy(other.key_start);
    key_finish.copy(other.key_finish);

    value.copy(other.value);

    ts_start.copy(other.ts_start);
    ts_finish.copy(other.ts_finish);

    flags.copy(other.flags);
    offset_key.copy(other.offset_key);
    offset_rev = other.offset_rev;
  }

  virtual ~Interval(){
    //std::cout << " ~Interval\n";
    free();
  }
  
  void free() {
    range_begin.free();
    range_end.free();
    key_start.free();
    key_finish.free();
    value.free();
    offset_key.free();
  }
/*
  void expand(const Cells::Cell& cell) {
    if(key_start.empty() || !key_start.is_matching(cell.key)){
      key_start.set(cell.key, Condition::GE);
    }
    if(key_finish.empty() || !key_finish.is_matching(cell.key)){
      key_finish.set(cell.key, Condition::LE);
    }
  }
*/
  bool equal(const Interval& other) const {
    return  ts_start.equal(other.ts_start) &&
            ts_finish.equal(other.ts_finish) &&
            flags.equal(other.flags) &&
            range_begin.equal(other.range_begin) &&
            range_end.equal(other.range_end) &&
            key_start.equal(other.key_start) &&
            key_finish.equal(other.key_finish) &&
            value.equal(other.value) &&
            offset_key.equal(offset_key) &&
            offset_rev == offset_rev ;
  }

  const bool is_matching(const Cell::Key& key, 
                         int64_t timestamp, bool desc) const {
    if(offset_key.empty()) 
      return true;

    switch(offset_key.compare(key)) {
      case Condition::LT:
        return false;
      case Condition::EQ:
        return desc ? offset_rev > timestamp : offset_rev < timestamp;
      default:
        return true;
    }
  }

  const bool is_matching(const Cells::Cell& cell) const {
    bool match = is_matching(
      cell.key, cell.timestamp, cell.control & Cells::TS_DESC);
    if(!match)
      return match;

    return  
      ts_start.is_matching(cell.timestamp) 
      &&
      ts_finish.is_matching(cell.timestamp) 
      &&
      (range_begin.empty() || range_begin.compare(cell.key) != Condition::LT) 
      &&
      (range_end.empty()   || range_end.compare(cell.key) != Condition::GT) 
      &&
      (key_start.empty()   || key_start.is_matching(cell.key)) 
      &&
      (key_finish.empty()  || key_finish.is_matching(cell.key))
      ;
  }

  const bool is_matching(const Cells::Cell& cell, Types::Column typ) const {
    bool match = is_matching(cell);
    if(!match || value.empty())
      return match;

    switch(typ) {
      case Types::Column::COUNTER_I64: 
        return value.is_matching(cell.get_counter());
      default:
        return value.is_matching(cell.value, cell.vlen);
    }
  }

  const size_t encoded_length() const {
    return range_begin.encoded_length() + range_end.encoded_length()
          + key_start.encoded_length() + key_finish.encoded_length()
          + value.encoded_length()
          + ts_start.encoded_length() + ts_finish.encoded_length()
          + flags.encoded_length()
          + offset_key.encoded_length()
          + Serialization::encoded_length_vi64(offset_rev);
  }

  void encode(uint8_t **bufp) const {
    range_begin.encode(bufp);
    range_end.encode(bufp);

    key_start.encode(bufp);
    key_finish.encode(bufp);

    value.encode(bufp);

    ts_start.encode(bufp);
    ts_finish.encode(bufp);

    flags.encode(bufp);

    offset_key.encode(bufp);
    Serialization::encode_vi64(bufp, offset_rev);
  }

  void decode(const uint8_t **bufp, size_t *remainp){
    range_begin.decode(bufp, remainp);
    range_end.decode(bufp, remainp);

    key_start.decode(bufp, remainp);
    key_finish.decode(bufp, remainp);

    value.decode(bufp, remainp);

    ts_start.decode(bufp, remainp);
    ts_finish.decode(bufp, remainp);

    flags.decode(bufp, remainp);

    offset_key.decode(bufp, remainp);
    offset_rev = Serialization::decode_vi64(bufp, remainp);
  }
  
  const std::string to_string() const {
    std::string s("Interval(Begin");
    s.append(range_begin.to_string());
    s.append(" End");
    s.append(range_end.to_string());

    s.append(" Start");
    s.append(key_start.to_string());
    s.append(" Finish");
    s.append(key_finish.to_string());
    s.append(" ");

    s.append(" Start");
    s.append(ts_start.to_string());
    s.append(" Finish");
    s.append(ts_finish.to_string());

    s.append(" Offset");
    s.append(offset_key.to_string());
    s.append(" OffsetRev=");
    s.append(std::to_string(offset_rev));

    s.append(value.to_string());

    s.append(" ");
    s.append(flags.to_string());

    return s;
  }

  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const {
    out << offset << "Interval(\n"; 

    out << offset << " Range(\n"
        << offset << "   begin(";
    range_begin.display_details(out);
    out << ")\n"
        << offset << "     end(";
    range_end.display_details(out);
    out << ")\n"
        << offset << " )\n";

    out << offset << " Key(\n"
        << offset << "   start(";
    key_start.display(out);
    out << ")\n"
        << offset << "  finish(";
    key_finish.display(out);
    out << ")\n"
        << offset << " )\n";

    out << offset << " Timestamp(\n"
        << offset << "   start(";
    ts_start.display(out);
    out << ")\n"
        << offset << "  finish(";
    ts_finish.display(out);
    out << ")\n"
        << offset << " )\n";

    out << offset << " Offset(\n"
        << offset << "     key(";
    offset_key.display_details(out);
    out << ")\n"
        << offset << "     rev(" << offset_rev << " )\n"
        << offset << " )\n";

    out << offset << " Value(";
    value.display(out);
    out << ")\n"; 

    out << offset << " Flags(";
    flags.display(out);
    out << ")\n"; 

    out << offset << ")\n"; 
  }

  Cell::Key  range_begin, range_end;
  Key        key_start, key_finish;
  Value      value;
  Timestamp  ts_start, ts_finish;
  Flags      flags;

  Cell::Key  offset_key;
  int64_t    offset_rev;
};


}}}
#endif