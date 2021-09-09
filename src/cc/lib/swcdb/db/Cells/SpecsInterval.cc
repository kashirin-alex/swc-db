/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Interval::Interval(Types::Column col_type) noexcept
                  : values(col_type), offset_rev(0), options(0) {
}

Interval::Interval(const Cell::Key& range_begin, const Cell::Key& range_end)
                  : range_begin(range_begin), range_end(range_end),
                    offset_rev(0), options(0) {
}

Interval::Interval(const uint8_t** bufp, size_t* remainp) {
  decode(bufp, remainp);
}

Interval::Interval(const Interval& other)
                  : range_begin(other.range_begin),
                    range_end(other.range_end),
                    key_intervals(other.key_intervals),
                    values(other.values),
                    ts_start(other.ts_start),
                    ts_finish(other.ts_finish),
                    flags(other.flags),
                    offset_key(other.offset_key),
                    offset_rev(other.offset_rev),
                    options(other.options) {
}

Interval::Interval(Interval&& other) noexcept
                  : range_begin(std::move(other.range_begin)),
                    range_end(std::move(other.range_end)),
                    key_intervals(std::move(other.key_intervals)),
                    values(std::move(other.values)),
                    ts_start(std::move(other.ts_start)),
                    ts_finish(std::move(other.ts_finish)),
                    flags(std::move(other.flags)),
                    offset_key(std::move(other.offset_key)),
                    offset_rev(other.offset_rev),
                    options(other.options) {
}

Interval::~Interval() { }

void Interval::copy(const Interval& other) {
  range_begin.copy(other.range_begin);
  range_end.copy(other.range_end);

  key_intervals.copy(other.key_intervals);

  values.copy(other.values);

  ts_start.copy(other.ts_start);
  ts_finish.copy(other.ts_finish);

  flags.copy(other.flags);

  offset_key.copy(other.offset_key);
  offset_rev = other.offset_rev;

  options = other.options;
}

void Interval::move(Interval& other) noexcept {
  range_begin.move(other.range_begin);
  range_end.move(other.range_end);

  key_intervals.move(other.key_intervals);

  values.move(other.values);

  ts_start.copy(other.ts_start);
  ts_finish.copy(other.ts_finish);

  flags.copy(other.flags);

  offset_key.move(other.offset_key);
  offset_rev = other.offset_rev;

  options = other.options;
}

void Interval::free() {
  range_begin.free();
  range_end.free();
  key_intervals.clear();
  values.clear();
  offset_key.free();
}

bool Interval::equal(const Interval& other) const noexcept {
  return  ts_start.equal(other.ts_start) &&
          ts_finish.equal(other.ts_finish) &&
          flags.equal(other.flags) &&
          options == other.options &&
          range_begin.equal(other.range_begin) &&
          range_end.equal(other.range_end) &&
          key_intervals.equal(other.key_intervals) &&
          values.equal(other.values) &&
          offset_key.equal(other.offset_key) &&
          offset_rev == other.offset_rev ;
}


size_t Interval::encoded_length() const noexcept {
  return range_begin.encoded_length() + range_end.encoded_length()
        + key_intervals.encoded_length()
        + values.encoded_length()
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

  values.encode(bufp);

  ts_start.encode(bufp);
  ts_finish.encode(bufp);

  flags.encode(bufp);

  offset_key.encode(bufp);
  Serialization::encode_vi64(bufp, offset_rev);
  Serialization::encode_i8(bufp, options);
}

void Interval::decode(const uint8_t** bufp, size_t* remainp, bool owner) {
  range_begin.decode(bufp, remainp, owner);
  range_end.decode(bufp, remainp, owner);

  key_intervals.decode(bufp, remainp);

  values.decode(bufp, remainp, owner);

  ts_start.decode(bufp, remainp);
  ts_finish.decode(bufp, remainp);

  flags.decode(bufp, remainp);

  offset_key.decode(bufp, remainp, owner);
  offset_rev = Serialization::decode_vi64(bufp, remainp);
  options = Serialization::decode_i8(bufp, remainp);
}

void Interval::apply_possible_range_begin(DB::Cell::Key& begin) const {
  if(!offset_key.empty()) {
    begin.copy(offset_key);

  } else if(!range_begin.empty()) {
    if(&begin != &range_begin)
      begin.copy(range_begin);

  } else if(!key_intervals.empty()) {
    begin.free();
    apply_possible_range(begin, false, false, false);
  }
}

void Interval::apply_possible_range_end(DB::Cell::Key& end,
                                        bool* restp) const {
  if(!range_end.empty()) {
    if(&end != &range_end)
      end.copy(range_end);

  } else if(!key_intervals.empty()) {
    end.free();
    apply_possible_range(end, true, restp, false);
    if(restp && !end.empty())
      *restp = true;
  }
}

void Interval::apply_possible_range(DB::Cell::Key& key, bool ending,
                                    bool rest, bool no_stepping) const {
  size_t sz = 0;
  for(const auto& intval : key_intervals) {
    if(sz < intval.start.size())
      sz = intval.start.size();
    if(sz < intval.finish.size())
      sz = intval.finish.size();
  }
  std::vector<std::string> key_range(sz + (ending && !rest));
  bool initial = true;
  bool found = false;
  do_:
    for(const auto& intval : key_intervals) {
      const auto* keyp = ending
        ? (initial ? &intval.finish : &intval.start )
        : (initial ? &intval.start  : &intval.finish);
      for(size_t idx=0; idx < keyp->size(); ++idx) {
        auto& key_f = key_range[idx];
        if(!key_f.empty())
          continue;
        const Fraction& f = (*keyp)[idx];
        if(f.empty())
          continue;
        switch(f.comp) {
          case Condition::EQ:
            break;
          case Condition::LE:
          case Condition::LT:
            if(!ending)
              continue;
            break;
          case Condition::PF:
          case Condition::GT:
          case Condition::GE:
            if(ending)
              continue;
            break;
          default:
            continue;
        }
        key_f.append(f.data(), f.size());
        found = true;
      }
    }
    if(initial) {
      initial = false;
      goto do_;
    }
  if(!found)
    return;

  if(no_stepping) {
    for(auto it = key_range.cbegin(); it != key_range.cend(); ++it) {
      if(it->empty()) {
        key.add(key_range.cbegin(), it);
        return;
      }
    }
    key.add(key_range.cbegin(), key_range.cend());

  } else {
    auto it = key_range.cend();
    do {
      --it;
      if(!it->empty()) {
        if(ending && !rest)
          ++it;
        key.add(key_range.cbegin(), ++it);
        break;
      }
    } while(it != key_range.cbegin());
  }
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

  values.print(out << ' ');

  flags.print(out << ' ');

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

  values.display(out, pretty, offset + ' ');

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
