/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Interval::Interval(Types::Column col_type) noexcept
                  : values(col_type), offset_rev(0), options(0) { }

Interval::Interval(const Cell::Key& range_begin, const Cell::Key& range_end)
                  : range_begin(range_begin), range_end(range_end),
                    offset_rev(0), options(0) {
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

  values.copy(other.values);

  ts_start.copy(other.ts_start);
  ts_finish.copy(other.ts_finish);

  flags.copy(other.flags);

  offset_key.copy(other.offset_key);
  offset_rev = other.offset_rev;

  options = other.options;
}

Interval::~Interval() {
  free();
}

void Interval::free() {
  range_begin.free();
  range_end.free();
  key_intervals.free();
  values.free();
  offset_key.free();
}

size_t Interval::size_of_internal() const noexcept {
  return range_begin.size + range_end.size
        + key_intervals.size_of_internal()
        + values.size_of_internal()
        + offset_key.size;
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

bool Interval::is_matching(int64_t timestamp, bool desc) const noexcept {
  return desc ? offset_rev > timestamp : offset_rev < timestamp;
}

bool Interval::is_matching(const Types::KeySeq key_seq,
                           const Cells::Cell& cell, bool& stop) const {
  return
    is_matching(
      key_seq, cell.key, cell.timestamp, cell.control & Cells::TS_DESC)
    &&
    ts_start.is_matching(cell.timestamp)
    &&
    ts_finish.is_matching(cell.timestamp)
    &&
    is_matching_begin(key_seq, cell.key)
    &&
    !(stop = !is_matching_end(key_seq, cell.key))
    &&
    key_intervals.is_matching(key_seq, cell.key)
    &&
    values.is_matching(cell);
}


bool Interval::is_matching_begin(const Types::KeySeq key_seq,
                                 const DB::Cell::Key& key) const {
  if(!range_begin.empty()) switch(key_seq) {

    case Types::KeySeq::LEXIC:
      return
        DB::KeySeq::compare_opt_lexic(
          range_begin, key, range_begin.count, true
        ) != Condition::LT;

    case Types::KeySeq::VOLUME:
      return
        DB::KeySeq::compare_opt_volume(
          range_begin, key, range_begin.count, true
        ) != Condition::LT;

    case Types::KeySeq::FC_LEXIC:
      return
        DB::KeySeq::compare_opt_fc_lexic(
          range_begin, key, key.count, true
        ) != Condition::LT;

    case Types::KeySeq::FC_VOLUME:
      return
        DB::KeySeq::compare_opt_fc_volume(
          range_begin, key, key.count, true
        ) != Condition::LT;

    default:
      break;
  }
  return true;
}

bool Interval::is_matching_end(const Types::KeySeq key_seq,
                               const DB::Cell::Key& key) const {
  if(!range_end.empty()) switch(key_seq) {

    case Types::KeySeq::LEXIC:
      return
        DB::KeySeq::compare_opt_lexic(
          range_end, key,
          has_opt__range_end_rest() && !has_opt__key_equal()
            ? range_end.count : key.count,
          true
        ) != Condition::GT;

    case Types::KeySeq::VOLUME:
      return
        DB::KeySeq::compare_opt_volume(
          range_end, key,
          has_opt__range_end_rest() && !has_opt__key_equal()
            ? range_end.count : key.count,
          true
        ) != Condition::GT;

    case Types::KeySeq::FC_LEXIC:
      return
        (has_opt__key_equal()
          ? key.count < range_end.count
          : has_opt__range_end_rest()) ||
        DB::KeySeq::compare_opt_fc_lexic(
          range_end, key,
          has_opt__range_end_rest() && !has_opt__key_equal()
            ? range_end.count : key.count,
          true
        ) != Condition::GT;

    case Types::KeySeq::FC_VOLUME:
      return
        (has_opt__key_equal()
          ? key.count < range_end.count
          : has_opt__range_end_rest()) ||
        DB::KeySeq::compare_opt_fc_volume(
          range_end, key,
          has_opt__range_end_rest() && !has_opt__key_equal()
            ? range_end.count : key.count,
          true
        ) != Condition::GT;
    default:
      break;
  }
  return true;
}

bool Interval::is_in_previous(const Types::KeySeq key_seq,
                              const DB::Cell::Key& prev) const {
  if(!range_end.empty()) switch(key_seq) {

    case Types::KeySeq::LEXIC:
      return
        DB::KeySeq::compare_opt_lexic(
          range_end, prev,
          has_opt__range_end_rest()
            ? range_end.count : prev.count,
          true
        ) != Condition::GT;

    case Types::KeySeq::VOLUME:
      return
        DB::KeySeq::compare_opt_volume(
          range_end, prev,
          has_opt__range_end_rest()
            ? range_end.count : prev.count,
          true
        ) != Condition::GT;

    case Types::KeySeq::FC_LEXIC:
      return
        has_opt__range_end_rest()
        ? (prev.count >= range_end.count
            ? true
            : DB::KeySeq::compare_opt_lexic(
                range_end, prev, range_end.count, true
              ) != Condition::GT)
        : DB::KeySeq::compare_opt_fc_lexic(
            range_end, prev,
            has_opt__key_equal()
              ? range_end.count : prev.count,
            true
          ) != Condition::GT;

    case Types::KeySeq::FC_VOLUME:
      return
        has_opt__range_end_rest()
        ? (prev.count >= range_end.count
            ? true
            : DB::KeySeq::compare_opt_volume(
                range_end, prev, range_end.count, true
              ) != Condition::GT)
        : DB::KeySeq::compare_opt_fc_volume(
            range_end, prev,
            has_opt__key_equal()
              ? range_end.count : prev.count,
            true
          ) != Condition::GT;

    default:
      break;
  }
  return true;
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

void Interval::decode(const uint8_t** bufp, size_t* remainp) {
  range_begin.decode(bufp, remainp, false);
  range_end.decode(bufp, remainp, false);

  key_intervals.decode(bufp, remainp);

  values.decode(bufp, remainp);

  ts_start.decode(bufp, remainp);
  ts_finish.decode(bufp, remainp);

  flags.decode(bufp, remainp);

  offset_key.decode(bufp, remainp, false);
  offset_rev = Serialization::decode_vi64(bufp, remainp);
  options = Serialization::decode_i8(bufp, remainp);
}


void Interval::set_opt__key_equal() noexcept {
  options |= OPT_KEY_EQUAL;
}

void Interval::set_opt__range_end_rest() noexcept {
  options |= OPT_RANGE_END_REST;
}

bool Interval::has_opt__key_equal() const noexcept {
  return options & OPT_KEY_EQUAL;
}

bool Interval::has_opt__range_end_rest() const noexcept {
  return options & OPT_RANGE_END_REST;
}


void Interval::apply_possible_range_pure() {
  if(key_intervals.empty())
    return;

  if(range_begin.empty()) {
    apply_possible_range(range_begin, false, false, true);
  }
  if(range_end.empty()) {
    apply_possible_range(range_end, true, true, true);
    if(!range_end.empty() && !has_opt__key_equal())
      set_opt__range_end_rest();
  }
}

void Interval::apply_possible_range(DB::Cell::Key& begin, DB::Cell::Key& end,
                                    bool* end_restp) const {
  apply_possible_range_begin(begin);
  apply_possible_range_end(end, end_restp);
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
    if(sz < intval->start.size())
      sz = intval->start.size();
    if(sz < intval->finish.size())
      sz = intval->finish.size();
  }
  std::vector<std::string> key_range(sz + (ending && !rest));
  bool initial = true;
  bool found = false;
  do_:
    for(auto& intval : key_intervals) {
      auto* keyp = ending
        ? (initial ? &intval->finish : &intval->start )
        : (initial ? &intval->start  : &intval->finish);
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
