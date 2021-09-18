/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Interval_h
#define swcdb_db_Cells_Interval_h

#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC {  namespace DB { namespace Cells {

class Interval final {

  /* encoded-format:
      key_begin-encoded key_end-encoded vi64(ts_earliest) vi64(ts_latest)
  */

  public:

  const Types::KeySeq key_seq;

  SWC_CAN_INLINE
  explicit Interval(const Types::KeySeq a_key_seq) noexcept
                    : key_seq(a_key_seq) {
  }

  SWC_CAN_INLINE
  explicit Interval(const Types::KeySeq a_key_seq,
                    const uint8_t **ptr, size_t *remain)
                  : key_seq(a_key_seq) {
    decode(ptr, remain, true);
  }

  SWC_CAN_INLINE
  explicit Interval(const Interval& other)
                  : key_seq(other.key_seq) {
    copy(other);
  }

  SWC_CAN_INLINE
  explicit Interval(Interval&& other) noexcept
                    : key_seq(other.key_seq),
                      key_begin(std::move(other.key_begin)),
                      key_end(std::move(other.key_end)),
                      ts_earliest(std::move(other.ts_earliest)),
                      ts_latest(std::move(other.ts_latest)),
                      aligned_min(std::move(other.aligned_min)),
                      aligned_max(std::move(other.aligned_max)),
                      was_set(other.was_set) {
  }

  Interval& operator=(const Interval&) = delete;

  ~Interval() noexcept;

  void copy(const Interval& other);

  void free() noexcept;

  SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    return key_begin.size + key_end.size +
           aligned_min.size_of_internal() +
           aligned_min.size_of_internal();
  }

  SWC_CAN_INLINE
  void set_key_begin(const DB::Cell::Key& key) {
    key_begin.copy(key);
    was_set = true;
  }

  SWC_CAN_INLINE
  void set_key_end(const DB::Cell::Key& key) {
    key_end.copy(key);
    was_set = true;
  }

  constexpr SWC_CAN_INLINE
  void set_ts_earliest(const Specs::Timestamp& ts) {
    ts_earliest.copy(ts);
    was_set = true;
  }

  constexpr SWC_CAN_INLINE
  void set_ts_latest(const Specs::Timestamp& ts) {
    ts_latest.copy(ts);
    was_set = true;
  }

  SWC_CAN_INLINE
  void set_aligned_min(const DB::Cell::KeyVec& key) {
    aligned_min.copy(key);
    was_set = true;
  }

  SWC_CAN_INLINE
  void set_aligned_max(const DB::Cell::KeyVec& key) {
    aligned_max.copy(key);
    was_set = true;
  }

  void expand(const Interval& other);

  void expand(const Cell& cell);

  SWC_CAN_INLINE
  void expand_begin(const Cell& cell) {
    if(key_begin.empty() || !is_in_begin(cell.key))
      key_begin.copy(cell.key);
    was_set = true;
  }

  SWC_CAN_INLINE
  void expand_end(const Cell& cell) {
    if(key_end.empty() || !is_in_end(cell.key))
      key_end.copy(cell.key);
    was_set = true;
  }

  SWC_CAN_INLINE
  void expand(const int64_t& ts) {
    if(ts_earliest.empty() || !ts_earliest.is_matching(ts))
      ts_earliest.set(ts, Condition::GE);
    if(ts_latest.empty() || !ts_latest.is_matching(ts))
      ts_latest.set(ts, Condition::LE);
    was_set = true;
  }

  SWC_CAN_INLINE
  bool align(const Interval &other) {
    return align(other.aligned_min, other.aligned_max);
  }

  SWC_CAN_INLINE
  bool align(const DB::Cell::KeyVec& _min, const DB::Cell::KeyVec& _max) {
    bool start = DB::KeySeq::align(key_seq, aligned_min, _min, Condition::LT);
    bool finish = DB::KeySeq::align(key_seq, aligned_max, _max, Condition::GT);
    return start || finish;
  }

  SWC_CAN_INLINE
  bool align(const DB::Cell::Key &key) {
    return DB::KeySeq::align(key_seq, key, aligned_min, aligned_max);
  }

  bool equal(const Interval& other) const noexcept;

  SWC_CAN_INLINE
  bool is_in_begin(const DB::Cell::Key &key) const {
    return key_begin.empty() ||
          (!key.empty() &&
            DB::KeySeq::compare(key_seq, key_begin, key) != Condition::LT);
  }

  SWC_CAN_INLINE
  bool is_in_end(const DB::Cell::Key &key) const {
    return key_end.empty() ||
          (!key.empty() &&
            DB::KeySeq::compare(key_seq, key_end, key) != Condition::GT);
  }

  /*
  bool consist(const Interval& other) const;

  bool consist(const DB::Cell::Key& key) const;

  bool consist(const DB::Cell::Key& key, int64_t ts) const;

  SWC_CAN_INLINE
  bool includes(const Interval& other) const {
    return other.key_begin.empty() || other.key_end.empty() || consist(other);
  }

  bool includes_begin(const Specs::Interval& interval) const;

  bool includes_end(const Specs::Interval& interval) const;

  bool includes(const Specs::Interval& interval) const;
  */

  size_t encoded_length() const noexcept;

  void encode(uint8_t **ptr) const;

  void decode(const uint8_t **ptr, size_t *remain, bool owner);

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const Interval& intval) {
    intval.print(out);
    return out;
  }

  DB::Cell::Key     key_begin;
  DB::Cell::Key     key_end;
  Specs::Timestamp  ts_earliest;
  Specs::Timestamp  ts_latest;
  DB::Cell::KeyVec  aligned_min;
  DB::Cell::KeyVec  aligned_max;
  bool              was_set = false;

};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Interval.cc"
#endif

#endif // swcdb_db_Cells_Interval_h
