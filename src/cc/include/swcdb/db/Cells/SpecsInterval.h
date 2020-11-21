/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsInterval_h
#define swcdb_db_cells_SpecsInterval_h


#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/db/Cells/SpecsKeyIntervals.h"
#include "swcdb/db/Cells/SpecsTimestamp.h"
#include "swcdb/db/Cells/SpecsValue.h"
#include "swcdb/db/Cells/SpecsFlags.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Specs {

class Interval {
  public:

  typedef std::shared_ptr<Interval> Ptr;

  static Ptr make_ptr();

  static Ptr make_ptr(
      const Key& key_start, const Key& key_finish, const Value& value,
      const Timestamp& ts_start, const Timestamp& ts_finish, 
      const Flags& flags=Flags());
  
  static Ptr make_ptr(
      const Cell::Key& range_begin, const Cell::Key& range_end,
      const Key& key_start, const Key& key_finish, 
      const Value& value, 
      const Timestamp& ts_start, const Timestamp& ts_finish, 
      const Flags& flags=Flags());
  
  static Ptr make_ptr(const uint8_t** bufp, size_t* remainp);

  static Ptr make_ptr(const Interval& other);

  static Ptr make_ptr(Ptr other);
  

  explicit Interval();

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end);

  explicit Interval(const Key& key_start, const Key& key_finish, 
                    const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags());

  explicit Interval(const Cell::Key& range_begin, const Cell::Key& range_end, 
                    const Key& key_start, const Key& key_finish, 
                    const Value& value, 
                    const Timestamp& ts_start, const Timestamp& ts_finish, 
                    const Flags& flags=Flags());
  
  explicit Interval(const uint8_t** bufp, size_t* remainp);

  explicit Interval(const Interval& other);

  void copy(const Interval& other);

  ~Interval();
  
  void free() ;

  size_t size_of_internal() const;

  //void expand(const Cells::Cell& cell);

  bool equal(const Interval& other) const;

  bool is_matching(const Types::KeySeq key_seq, 
                   const Cell::Key& key, 
                   int64_t timestamp, bool desc) const;

  bool is_matching(int64_t timestamp, bool desc) const;

  bool is_matching(const Types::KeySeq key_seq, 
                   const Cells::Cell& cell) const;

  bool is_matching_begin(const Types::KeySeq key_seq, 
                         const DB::Cell::Key& key) const;

  bool is_matching_end(const Types::KeySeq key_seq, 
                       const DB::Cell::Key& key) const;

  size_t encoded_length() const;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);
  
  void apply_possible_range(DB::Cell::Key& begin, DB::Cell::Key& end) const;

  void apply_possible_range_begin(DB::Cell::Key& begin) const;

  void apply_possible_range_end(DB::Cell::Key& end) const;

  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=false, 
               const std::string& offset = "") const;

  Cell::Key     range_begin, range_end, range_offset;
  KeyIntervals  key_intervals;
  Value         value;
  Timestamp     ts_start, ts_finish;
  Flags         flags;

  bool          key_eq;

  Cell::Key     offset_key;
  int64_t       offset_rev;
  Types::Column col_type;
};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsInterval.cc"
#endif 

#endif // swcdb_db_cells_SpecsInterval_h
