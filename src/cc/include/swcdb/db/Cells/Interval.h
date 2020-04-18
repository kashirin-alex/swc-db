/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

  const KeyComp* key_comp;
  
  explicit Interval(const Types::KeySeq seq);

  explicit Interval(const KeyComp* key_comp);

  explicit Interval(const KeyComp* key_comp,
                    const uint8_t **ptr, size_t *remain);

  explicit Interval(const Interval& other);

  ~Interval();

  void copy(const Interval& other);

  void free();
  
  void set_key_begin(const DB::Cell::Key& key);

  void set_key_end(const DB::Cell::Key& key);

  void set_ts_earliest(const Specs::Timestamp& ts);

  void set_ts_latest(const Specs::Timestamp& ts);

  void set_aligned_min(const DB::Cell::KeyVec& key);

  void set_aligned_max(const DB::Cell::KeyVec& key);

  void expand(const Interval& other);

  void expand(const Cell& cell);

  void expand_begin(const Cell& cell);

  void expand_end(const Cell& cell);

  void expand(const int64_t& ts);

  bool align(const Interval &other);

  bool equal(const Interval& other) const;

  bool is_in_begin(const DB::Cell::Key &key) const;
  
  bool is_in_end(const DB::Cell::Key &key) const;

  bool consist(const Interval& other) const;

  bool consist(const DB::Cell::Key& key) const;

  bool consist(const DB::Cell::Key& key, int64_t ts) const;

  bool includes(const Interval& other) const;

  bool includes_begin(const Specs::Interval& interval) const;

  bool includes_end(const Specs::Interval& interval) const;

  bool includes(const Specs::Interval& interval) const;

  size_t encoded_length() const;

  void encode(uint8_t **ptr) const;

  void decode(const uint8_t **ptr, size_t *remain, bool owner=false);

  std::string to_string() const;


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