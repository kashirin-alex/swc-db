/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Interval::Ptr Interval::make_ptr() {
  return std::make_shared<Interval>();
}

Interval::Ptr Interval::make_ptr(
    const Key& key_start, const Key& key_finish, const Value& value,
    const Timestamp& ts_start, const Timestamp& ts_finish, 
    const Flags& flags) {
  return std::make_shared<Interval>(
    key_start, key_finish, value, ts_start, ts_finish, flags);
}

Interval::Ptr Interval::make_ptr(
    const Cell::Key& range_begin, const Cell::Key& range_end,
    const Key& key_start, const Key& key_finish, 
    const Value& value, 
    const Timestamp& ts_start, const Timestamp& ts_finish, 
    const Flags& flags) {
  return std::make_shared<Interval>(
    range_begin, range_end, 
    key_start, key_finish, value, ts_start, ts_finish, flags
    );
}

Interval::Ptr Interval::make_ptr(const uint8_t** bufp, size_t* remainp) {
  return std::make_shared<Interval>(bufp, remainp);
}

Interval::Ptr Interval::make_ptr(const Interval& other) {
  return std::make_shared<Interval>(other);
}

Interval::Ptr Interval::make_ptr(Interval::Ptr other) {
  return std::make_shared<Interval>(*other.get());
}


Interval::Interval() : offset_rev(0), options(0) { }

Interval::Interval(const Cell::Key& range_begin, const Cell::Key& range_end)
                  : range_begin(range_begin), range_end(range_end), 
                    offset_rev(0), options(0) {
}

Interval::Interval(const Key& key_start, const Key& key_finish, 
                   const Value& value, 
                   const Timestamp& ts_start, const Timestamp& ts_finish, 
                   const Flags& flags)
                  : value(value),
                    ts_start(ts_start), ts_finish(ts_finish), 
                    flags(flags), offset_rev(0), options(0) {
  key_intervals.add(key_start, key_finish);
}

Interval::Interval(const Cell::Key& range_begin, const Cell::Key& range_end, 
                   const Key& key_start, const Key& key_finish, 
                   const Value& value, 
                   const Timestamp& ts_start, const Timestamp& ts_finish, 
                   const Flags& flags)
                  : range_begin(range_begin), range_end(range_end), 
                    value(value),
                    ts_start(ts_start), ts_finish(ts_finish), 
                    flags(flags), offset_rev(0), options(0) {
  key_intervals.add(key_start, key_finish);
}

Interval::Interval(const uint8_t** bufp, size_t* remainp) {
  decode(bufp, remainp); 
}

Interval::Interval(const Interval& other) {
  copy(other);
}

void Interval::copy(const Interval& other) {

  range_begin.copy(other.range_begin);
  range_end.copy(other.range_end);

  key_intervals.copy(other.key_intervals);

  value.copy(other.value);

  ts_start.copy(other.ts_start);
  ts_finish.copy(other.ts_finish);

  flags.copy(other.flags);

  offset_key.copy(other.offset_key);
  offset_rev = other.offset_rev;

  options = other.options;
}

Interval::~Interval(){
  free();
}

void Interval::free() {
  range_begin.free();
  range_end.free();
  key_intervals.free();
  value.free();
  offset_key.free();
}

size_t Interval::size_of_internal() const {
  return range_begin.size + range_end.size
        + key_intervals.size_of_internal()
        + value.size
        + offset_key.size;
}

/*
void Interval::expand(const Cells::Cell& cell) {
  if(key_start.empty() || !key_start.is_matching(cell.key)){
    key_start.set(cell.key, Condition::GE);
  }
  if(key_finish.empty() || !key_finish.is_matching(cell.key)){
    key_finish.set(cell.key, Condition::LE);
  }
}
*/

bool Interval::equal(const Interval& other) const {
  return  ts_start.equal(other.ts_start) &&
          ts_finish.equal(other.ts_finish) &&
          flags.equal(other.flags) &&
          options == other.options &&
          range_begin.equal(other.range_begin) &&
          range_end.equal(other.range_end) &&
          key_intervals.equal(other.key_intervals) &&
          value.equal(other.value) &&
          offset_key.equal(other.offset_key) &&
          offset_rev == other.offset_rev ;
}

bool Interval::is_matching(const Types::KeySeq key_seq, 
                           const Cell::Key& key, int64_t timestamp, 
                           bool desc) const {
  if(offset_key.empty()) 
    return true;

  switch(DB::KeySeq::compare(key_seq, offset_key, key)) {
    case Condition::LT:
      return false;
    case Condition::EQ:
      return is_matching(timestamp, desc);
    default:
      return true;
  }
}

bool Interval::is_matching(int64_t timestamp, bool desc) const {
  return desc ? offset_rev > timestamp : offset_rev < timestamp;
}

bool Interval::is_matching(const Types::KeySeq key_seq, 
                           const Cells::Cell& cell) const {
  bool match = is_matching(
    key_seq, cell.key, cell.timestamp, cell.control & Cells::TS_DESC);
  if(!match)
    return match;

  match =   
    ts_start.is_matching(cell.timestamp) 
    &&
    ts_finish.is_matching(cell.timestamp) 
    &&
    is_matching_begin(key_seq, cell.key)
    &&
    is_matching_end(key_seq, cell.key)
    &&
    key_intervals.is_matching(key_seq, cell.key);
  if(!match || value.empty())
    return match;

  if(Types::is_counter(col_type))
    return value.is_matching(cell.get_counter());
    
  return value.is_matching(cell.value, cell.vlen);
}


bool Interval::is_matching_begin(const Types::KeySeq key_seq, 
                                 const DB::Cell::Key& key) const {
  return range_begin.empty() || DB::KeySeq::compare_incl(
    key_seq, range_begin, key) != Condition::LT;
}

bool Interval::is_matching_end(const Types::KeySeq key_seq, 
                               const DB::Cell::Key& key) const {
  return range_end.empty() || DB::KeySeq::compare_incl(
    key_seq, range_end, key, has_opt__range_end_rest()) != Condition::GT;
}


size_t Interval::encoded_length() const {
  return range_begin.encoded_length() + range_end.encoded_length()
        + key_intervals.encoded_length()
        + value.encoded_length()
        + ts_start.encoded_length() + ts_finish.encoded_length()
        + flags.encoded_length()
        + offset_key.encoded_length()
        + Serialization::encoded_length_vi64(offset_rev)
        + 1;
}

void Interval::encode(uint8_t** bufp) const {
  range_begin.encode(bufp);
  range_end.encode(bufp);

  key_intervals.encode(bufp);

  value.encode(bufp);

  ts_start.encode(bufp);
  ts_finish.encode(bufp);

  flags.encode(bufp);

  offset_key.encode(bufp);
  Serialization::encode_vi64(bufp, offset_rev);
  Serialization::encode_i8(bufp, options);
}

void Interval::decode(const uint8_t** bufp, size_t* remainp) {
  range_begin.decode(bufp, remainp, false);
  range_end.decode(bufp, remainp, false);

  key_intervals.decode(bufp, remainp);

  value.decode(bufp, remainp);

  ts_start.decode(bufp, remainp);
  ts_finish.decode(bufp, remainp);

  flags.decode(bufp, remainp);

  offset_key.decode(bufp, remainp, false);
  offset_rev = Serialization::decode_vi64(bufp, remainp);
  options = Serialization::decode_i8(bufp, remainp);
}


void Interval::set_opt__key_equal() {
  options |= OPT_KEY_EQUAL;
}

void Interval::set_opt__range_end_rest() {
  options |= OPT_RANGE_END_REST;
}

bool Interval::has_opt__key_equal() const {
  return options & OPT_KEY_EQUAL;
}

bool Interval::has_opt__range_end_rest() const {
  return options & OPT_RANGE_END_REST;
}


void Interval::apply_possible_range(DB::Cell::Key& begin, 
                                    DB::Cell::Key& end) const {   
  apply_possible_range_begin(begin);
  apply_possible_range_end(end);
}

void Interval::apply_possible_range_begin(DB::Cell::Key& begin) const {
  if(!offset_key.empty()) {
    begin.copy(offset_key);
    
  } else if(!range_begin.empty()) {
    if(&begin != &range_begin)
      begin.copy(range_begin);
    
  } else if(!key_intervals.empty() && !key_intervals[0]->start.empty()) {
    std::string_view fraction;
    Condition::Comp comp;
    size_t ok;
    bool found = false;
    const Key& key_start(key_intervals[0]->start);
    for(size_t idx=0; idx < key_start.size(); ++idx) {
      fraction = key_start.get(idx, comp); // Fraction const-ref
      if(fraction.size() && 
        (comp == Condition::EQ || comp == Condition::PF || 
         comp == Condition::GT || comp == Condition::GE)) {
        begin.add(fraction);
        ok = idx;
        found = true;
      } else
        begin.add("", 0);
    }
    if(!found)
      begin.free();
    else if(++ok != key_start.size() && ok != begin.count)
      begin.remove(ok, true);

  } else if(!key_intervals.empty() && !key_intervals[0]->finish.empty()) {
    std::string_view fraction;
    Condition::Comp comp;
    size_t ok;
    bool found = false;
    const Key& key_finish(key_intervals[0]->finish);
    for(size_t idx=0; idx < key_finish.size(); ++idx) {
      fraction = key_finish.get(idx, comp);
      if(fraction.size() && 
        (comp == Condition::EQ || 
         comp == Condition::GT || comp == Condition::GE)) {
        begin.add(fraction);
        ok = idx;
        found = true;
      } else {
        begin.add("", 0);
      }
    }
    if(!found)
      begin.free();
    else if(++ok != key_finish.size() && ok != begin.count)
      begin.remove(ok, true);
  }
}

void Interval::apply_possible_range_end(DB::Cell::Key& end) const {
  if(!range_end.empty()) {
    if(&end != &range_end)
      end.copy(range_end);

  } else if(has_opt__key_equal() && 
            !key_intervals.empty() && 
            !key_intervals[0]->start.empty()) {
    std::string_view fraction;
    Condition::Comp comp;
    size_t ok;
    bool found = false;
    const Key& key_start(key_intervals[0]->start);
    for(size_t idx=0; idx < key_start.size(); ++idx) {
      fraction = key_start.get(idx, comp);
      if(fraction.length() && 
        (comp == Condition::LT || 
         comp == Condition::LE || 
         comp == Condition::EQ)) {
        end.add(fraction);
        ok = idx;
        found = true;
      } else {
        end.add("", 0);
        //options |= OPT_RANGE_END_REST;
      }
    }
    if(!found)
      end.free();
    else if(++ok == key_start.size())
      end.add("", 0); //options |= OPT_RANGE_END_REST;
    else if(++ok < key_start.size() && ok != end.count)
      end.remove(ok, true);
  }


  /* NOT POSSIBLE
    } else if(!key_finish.empty()) {
    const char* fraction;
    uint32_t len;
    Condition::Comp comp;
    int32_t ok = -1;
    for(int idx=0; idx < key_finish.size(); ++idx) {
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
    else if(++ok < key_finish.size() && ok != end.count)
      end.remove(ok, true);
      
  } else if(!key_start.empty()) {
    const char* fraction;
    uint32_t len;
    Condition::Comp comp;
    int32_t ok = -1;
    for(int idx=0; idx < key_start.size(); ++idx) {
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
    else if(ok == key_start.size())
      end.add("", 0);
    else if(++ok < key_start.size() && ok != end.count)
      end.remove(ok, true);
  }
  */
}

std::string Interval::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Interval::print(std::ostream& out) const {
  range_begin.print(out << "Interval(Begin");
  range_end.print(out << " End");

  key_intervals.print(out << ' ');

  ts_start.print(out << " Start");
  ts_finish.print(out << " Finish");

  offset_key.print(out << " Offset");
  if(!offset_key.empty())
    out << " OffsetRev=" << offset_rev;
  
  value.print(out << " ");
  flags.print(out << " ");
  
  out << " Options(" 
    << "range-end-rest=" << has_opt__range_end_rest()
    << " key-eq=" << has_opt__key_equal()
    << ')';
  out << ')';
}

void Interval::display(std::ostream& out, bool pretty, 
                       const std::string& offset) const {
  out << offset << "Interval(\n"; 

  out << offset << " Range(\n"
      << offset << "   begin(";
  range_begin.display_details(out, pretty);
  out << ")\n"
      << offset << "     end(";
  range_end.display_details(out, pretty);
  out << ")\n"
      << offset << " )\n";

  key_intervals.display(out, pretty, offset + ' ');

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
  offset_key.display_details(out, pretty);
  out << ")\n"
      << offset << "     rev(" << offset_rev << " )\n"
      << offset << " )\n";

  out << offset << " Value(";
  value.display(out, pretty);
  out << ")\n"; 

  out << offset << " Flags(";
  flags.display(out);
  out << ")\n"; 

  out << offset << " Options(" 
    << "range-end-rest=" << has_opt__range_end_rest()
    << " key-eq=" << has_opt__key_equal()
    << ")\n";

  out << offset << ")\n"; 
}



}}}
