/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsInterval_h
#define swcdb_db_cells_SpecsInterval_h


#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/db/Cells/SpecsKeyIntervals.h"
#include "swcdb/db/Cells/SpecsTimestamp.h"
#include "swcdb/db/Cells/SpecsValues.h"
#include "swcdb/db/Cells/SpecsFlags.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Specs {

class Interval {
  public:

  static constexpr const uint8_t OPT_KEY_EQUAL      = 0x01;
  static constexpr const uint8_t OPT_RANGE_END_REST = 0x02;

  typedef std::shared_ptr<Interval> Ptr;

  SWC_CAN_INLINE
  static Ptr make_ptr(Types::Column col_type = Types::Column::UNKNOWN) {
    return Ptr(new Interval(col_type));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const uint8_t** bufp, size_t* remainp) {
    return Ptr(new Interval(bufp, remainp));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const Interval& other) {
    return Ptr(new Interval(other));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(Interval&& other) {
    return Ptr(new Interval(std::move(other)));
  }


  explicit Interval(Types::Column col_type = Types::Column::UNKNOWN) noexcept;

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end);

  explicit Interval(const uint8_t** bufp, size_t* remainp);

  explicit Interval(const Interval& other);

  explicit Interval(Interval&& other) noexcept;

  SWC_CAN_INLINE
  Interval& operator=(const Interval& other) {
    copy(other);
    return *this;
  }

  SWC_CAN_INLINE
  Interval& operator=(Interval&& other) noexcept {
    move(other);
    return *this;
  }

  void copy(const Interval& other);

  void move(Interval& other) noexcept;

  ~Interval();

  void free();

  size_t size_of_internal() const noexcept;

  bool equal(const Interval& other) const noexcept;

  SWC_CAN_INLINE
  bool is_matching(const Types::KeySeq key_seq,
                   const Cell::Key& key,
                   int64_t timestamp, bool desc) const {
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

  constexpr SWC_CAN_INLINE
  bool is_matching(int64_t timestamp, bool desc) const noexcept {
    return desc ? offset_rev > timestamp : offset_rev < timestamp;
  }

  SWC_CAN_INLINE
  bool is_matching(const Types::KeySeq key_seq,
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

  bool is_matching_begin(const Types::KeySeq key_seq,
                         const DB::Cell::Key& key) const;

  bool is_matching_end(const Types::KeySeq key_seq,
                       const DB::Cell::Key& key) const;

  bool is_in_previous(const Types::KeySeq key_seq,
                      const DB::Cell::Key& prev) const;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp, bool owner=false);

  constexpr SWC_CAN_INLINE
  void set_opt__key_equal() noexcept {
    options |= OPT_KEY_EQUAL;
  }

  constexpr SWC_CAN_INLINE
  void set_opt__range_end_rest() noexcept {
    options |= OPT_RANGE_END_REST;
  }

  constexpr SWC_CAN_INLINE
  bool has_opt__key_equal() const noexcept {
    return options & OPT_KEY_EQUAL;
  }

  constexpr SWC_CAN_INLINE
  bool has_opt__range_end_rest() const noexcept {
    return options & OPT_RANGE_END_REST;
  }

  SWC_CAN_INLINE
  void apply_possible_range_pure() {
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

  SWC_CAN_INLINE
  void apply_possible_range(DB::Cell::Key& begin, DB::Cell::Key& end,
                            bool* end_restp = nullptr) const {
    apply_possible_range_begin(begin);
    apply_possible_range_end(end, end_restp);
  }

  void apply_possible_range_begin(DB::Cell::Key& begin) const;

  void apply_possible_range_end(DB::Cell::Key& end,
                                bool* restp = nullptr) const;

  void apply_possible_range(DB::Cell::Key& key, bool ending,
                            bool rest, bool no_stepping) const;

  SWC_CAN_INLINE
  std::string to_string() const {
    std::string s;
    {
      std::stringstream ss;
      print(ss);
      s = ss.str();
    }
    return s;
  }

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=false,
               const std::string& offset = "") const;

  Cell::Key     range_begin, range_end;
  KeyIntervals  key_intervals;
  Values        values;
  Timestamp     ts_start, ts_finish;
  Flags         flags;

  Cell::Key     offset_key;
  int64_t       offset_rev;

  uint8_t       options;

};



SWC_CAN_INLINE
size_t Interval::size_of_internal() const noexcept {
  return range_begin.size + range_end.size
        + key_intervals.size_of_internal()
        + values.size_of_internal()
        + offset_key.size;
}

SWC_CAN_INLINE
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

SWC_CAN_INLINE
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

SWC_CAN_INLINE
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


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsInterval.cc"
#endif

#endif // swcdb_db_cells_SpecsInterval_h
