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
  
  explicit Interval() : key_eq(false), offset_rev(0) {}

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end)
                    : range_begin(range_begin), range_end(range_end), 
                      key_eq(false), offset_rev(0) {
  }

  explicit Interval(const Key& key_start, const Key& key_finish, 
                    const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags())
                    : key_start(key_start), key_finish(key_finish), 
                      value(value),
                      ts_start(ts_start), ts_finish(ts_finish), 
                      flags(flags), key_eq(false), offset_rev(0) {
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
                      flags(flags), key_eq(false), offset_rev(0) {
  }
  
  explicit Interval(const uint8_t **bufp, size_t *remainp) 
                    : key_eq(false) {
    decode(bufp, remainp); 
  }

  explicit Interval(const Interval& other) {
    copy(other);
  }

  void copy(const Interval& other) {

    range_begin.copy(other.range_begin);
    range_end.copy(other.range_end);

    key_start.copy(other.key_start);
    key_finish.copy(other.key_finish);

    value.copy(other.value);

    ts_start.copy(other.ts_start);
    ts_finish.copy(other.ts_finish);

    flags.copy(other.flags);

    key_eq = other.key_eq;

    offset_key.copy(other.offset_key);
    offset_rev = other.offset_rev;
  }

  virtual ~Interval(){
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
        return is_matching(timestamp, desc);
      default:
        return true;
    }
  }

  const bool is_matching(int64_t timestamp, bool desc) const {
    return desc ? offset_rev > timestamp : offset_rev < timestamp;
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
      is_matching_begin(cell.key)
      &&
      is_matching_end(cell.key)
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

    if(Types::is_counter(typ))
      return value.is_matching(cell.get_counter());
      
    return value.is_matching(cell.value, cell.vlen);
  }

  const bool is_matching_begin(const DB::Cell::Key& key) const {
    return range_begin.empty() || 
           range_begin.compare(key, range_begin.count, true) != Condition::LT;
  }

  const bool is_matching_end(const DB::Cell::Key& key) const {
    return range_end.empty() || 
           range_end.compare(key, range_end.count, true) != Condition::GT;
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

  void decode(const uint8_t **bufp, size_t *remainp) {
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
  
  void apply_possible_range(DB::Cell::Key& begin, DB::Cell::Key& end) const {   
    apply_possible_range_begin(begin);
    apply_possible_range_end(end);
  }

  void apply_possible_range_begin(DB::Cell::Key& begin) const {
    if(!offset_key.empty()) {
      begin.copy(offset_key);
      
    } else if(!range_begin.empty()) {
      if(&begin != &range_begin)
        begin.copy(range_begin);
      
    } else if(!key_start.empty()) {

      const char* fraction;
      uint32_t len;
      Condition::Comp comp;
      int32_t ok = -1;
      for(int idx=0; idx < key_start.count; idx++) {
        key_start.get(idx, &fraction, &len, &comp);
        if(len && 
          (comp == Condition::EQ || comp == Condition::PF || 
           comp == Condition::GT || comp == Condition::GE)) {
          begin.add(fraction, len);
          ok = idx;
        } else
          begin.add("", 0);
      }
      if(!++ok)
        begin.free();
      else if(ok != key_start.count && ok != begin.count)
        begin.remove(ok, true);

    } else if(!key_finish.empty()) {
      const char* fraction;
      uint32_t len;
      Condition::Comp comp;
      int32_t ok = -1;
      for(int idx=0; idx < key_finish.count; idx++) {
        key_finish.get(idx, &fraction, &len, &comp);
        if(len && 
          (comp == Condition::EQ || 
           comp == Condition::GT || comp == Condition::GE)) {
          begin.add(fraction, len);
          ok = idx;
        } else
          begin.add("", 0);
      }
      if(!++ok)
        begin.free();
      else if(ok != key_finish.count && ok != begin.count)
        begin.remove(ok, true);
    }
  }

  void apply_possible_range_end(DB::Cell::Key& end) const {
    if(!range_end.empty()) {
      if(&end != &range_end)
        end.copy(range_end);

    } else if(key_eq && !key_start.empty()) {
      const char* fraction;
      uint32_t len;
      Condition::Comp comp;
      int32_t ok = -1;
      for(int idx=0; idx < key_start.count; idx++) {
        key_start.get(idx, &fraction, &len, &comp);
        if(len && 
          (comp == Condition::LT || 
           comp == Condition::LE || 
           comp == Condition::EQ)) {
          end.add(fraction, len);
          ok = idx;
        } else
          end.add("", 0);
      }
      if(!++ok)
        end.free();
      else if(ok == key_start.count)
        end.add("", 0);
      else if(++ok < key_start.count && ok != end.count)
        end.remove(ok, true);
    }


    /* NOT POSSIBLE
    } else if(!key_finish.empty()) {
      const char* fraction;
      uint32_t len;
      Condition::Comp comp;
      int32_t ok = -1;
      for(int idx=0; idx < key_finish.count; idx++) {
        key_finish.get(idx, &fraction, &len, &comp);
        if(len && 
          (comp == Condition::LT || comp == Condition::LE)) {
          end.add(fraction, len);
          ok = idx;
        } else 
          end.add("", 0);
      }
      if(!++ok)
        end.free();
      else if(++ok < key_finish.count && ok != end.count)
        end.remove(ok, true);
        
    } else if(!key_start.empty()) {
      const char* fraction;
      uint32_t len;
      Condition::Comp comp;
      int32_t ok = -1;
      for(int idx=0; idx < key_start.count; idx++) {
        key_start.get(idx, &fraction, &len, &comp);
        if(len && 
          (comp == Condition::LT || comp == Condition::LE)) {
          end.add(fraction, len);
          ok = idx;
        } else
          end.add("", 0);
      }
      if(!++ok)
        end.free();
      else if(ok == key_start.count)
        end.add("", 0);
      else if(++ok < key_start.count && ok != end.count)
        end.remove(ok, true);
    }
    */
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
    
    s.append(" ");
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

  Cell::Key  range_begin, range_end, range_offset;
  Key        key_start, key_finish;
  Value      value;
  Timestamp  ts_start, ts_finish;
  Flags      flags;

  bool       key_eq;

  Cell::Key  offset_key;
  int64_t    offset_rev;
};


}}}
#endif