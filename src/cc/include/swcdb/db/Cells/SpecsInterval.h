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

  static const uint8_t OPT_KEY_EQUAL      = 0x01;
  static const uint8_t OPT_RANGE_END_REST = 0x02;

  typedef std::shared_ptr<Interval> Ptr;

  SWC_CAN_INLINE
  static Ptr make_ptr(Types::Column col_type = Types::Column::UNKNOWN) {
    return std::make_shared<Interval>(col_type);
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const uint8_t** bufp, size_t* remainp) {
    return std::make_shared<Interval>(bufp, remainp);
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const Interval& other) {
    return std::make_shared<Interval>(other);
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(Interval&& other) {
    return std::make_shared<Interval>(std::move(other));
  }


  explicit Interval(Types::Column col_type = Types::Column::UNKNOWN) noexcept;

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end);

  explicit Interval(const uint8_t** bufp, size_t* remainp);

  explicit Interval(const Interval& other);

  explicit Interval(Interval&& other) noexcept;

  Interval& operator=(const Interval& other);

  Interval& operator=(Interval&& other) noexcept;

  void copy(const Interval& other);

  void move(Interval& other) noexcept;

  ~Interval();

  void free() ;

  size_t size_of_internal() const noexcept;

  bool equal(const Interval& other) const noexcept;

  bool is_matching(const Types::KeySeq key_seq,
                   const Cell::Key& key,
                   int64_t timestamp, bool desc) const;

  bool is_matching(int64_t timestamp, bool desc) const noexcept;

  bool is_matching(const Types::KeySeq key_seq,
                   const Cells::Cell& cell, bool& stop) const;

  bool is_matching_begin(const Types::KeySeq key_seq,
                         const DB::Cell::Key& key) const;

  bool is_matching_end(const Types::KeySeq key_seq,
                       const DB::Cell::Key& key) const;

  bool is_in_previous(const Types::KeySeq key_seq,
                      const DB::Cell::Key& prev) const;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  void set_opt__key_equal() noexcept;

  void set_opt__range_end_rest() noexcept;

  bool has_opt__key_equal() const noexcept;

  bool has_opt__range_end_rest() const noexcept;

  void apply_possible_range_pure();

  void apply_possible_range(DB::Cell::Key& begin, DB::Cell::Key& end,
                             bool* end_restp = nullptr) const;

  void apply_possible_range_begin(DB::Cell::Key& begin) const;

  void apply_possible_range_end(DB::Cell::Key& end,
                                bool* restp = nullptr) const;

  void apply_possible_range(DB::Cell::Key& key, bool ending,
                            bool rest, bool no_stepping) const;

  std::string to_string() const;

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


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsInterval.cc"
#endif

#endif // swcdb_db_cells_SpecsInterval_h
