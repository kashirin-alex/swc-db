/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h

#include <memory>
#include <functional>

#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/Interval.h"
#include "swcdb/db/Cells/Vector.h"


namespace SWC { namespace DB { namespace Cells {

typedef Vector Mutable;

/*
class Mutable final {

  static const uint32_t narrow_sz = 20;

  public:

  typedef std::shared_ptr<Mutable>                 Ptr;
  typedef std::function<bool(const Cell&, bool&)>  Selector_t;
  
  static const uint8_t  _cell_sz = sizeof(Cell*);
  
  Types::Column     type;

  static Ptr make(const uint32_t max_revs=1, 
                  const uint64_t ttl_ns=0, 
                  const Types::Column type=Types::Column::PLAIN);

  explicit Mutable(const uint32_t max_revs=1, 
                   const uint64_t ttl_ns=0, 
                   const Types::Column type=Types::Column::PLAIN);

  explicit Mutable(Mutable& other);

  void reset(const uint32_t max_revs=1, const uint32_t ttl=0, 
             const Types::Column typ=Types::Column::PLAIN);

  Mutable& operator=(const Mutable& other) = delete;

  ~Mutable();

  void configure(const uint32_t max_revs=1, 
                 const uint64_t ttl=0, 
                 const Types::Column typ=Types::Column::PLAIN);

  void ensure(uint32_t sz);

  void reserve(uint32_t sz);

  void free();
  
  const uint32_t size() const;
  
  const uint32_t size_bytes() const;
  
  void get(int32_t idx, Cell& cell) const;

  void get(int32_t idx, DB::Cell::Key& key) const;

  void get(int32_t idx, DB::Cell::Key& key, int64_t& ts) const;
   
  const bool get(const DB::Cell::Key& key, Condition::Comp comp, 
                 DB::Cell::Key& res) const;

  const bool get(const DB::Cell::Key& key, Condition::Comp comp, 
                 Cell& cell) const;

  bool get(const Specs::Key& key, Cell& cell) const;

  const Condition::Comp compare(int32_t idx, const DB::Cell::Key& key) const;

  const bool has_one_key() const;

  void add_sorted(const Cell& cell, bool no_value=false);

  void add_sorted_nocpy(Cell* cell);
  
  void insert(uint32_t offset, const Cell& cell);

  void add_raw(const Cell& e_cell);

  const bool get_next(uint32_t offset, Cell*& cell);

  void add_remove(const Cell& e_cell, uint32_t offset);

  void add_plain(const Cell& e_cell, uint32_t offset);

  void add_counter(const Cell& e_cell, uint32_t offset);

  void pop(int32_t idx, Cell& cell);

  void scan(Interval& interval, Mutable& cells) const;

  void scan(const Specs::Interval& specs, Vector& cells, 
            size_t& cell_offset, const std::function<bool()>& reached_limits, 
            size_t& skips, const Selector_t& selector) const;

  void scan_version_single(const Specs::Interval& specs, Vector& cells, 
                           size_t& cell_offset, 
                           const std::function<bool()>& reached_limits, 
                           size_t& skips, const Selector_t& selector) const;

  void scan_version_multi(const Specs::Interval& specs, Vector& cells, 
                          size_t& cell_offset, 
                          const std::function<bool()>& reached_limits, 
                          size_t& skips, const Selector_t& selector) const;
  

  void scan_test_use(const Specs::Interval& specs, DynamicBuffer& result, 
            size_t& count, size_t& skips) const;

  void write(DynamicBuffer& cells) const;

  void write_and_free(DynamicBuffer& cells, 
                      uint32_t& cell_count, Interval& intval, 
                      uint32_t threshold, uint32_t max_cells);
  
  bool write_and_free(const DB::Cell::Key& key_start, 
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);
  
  void split(uint32_t from, Mutable& cells, 
             Interval& intval_1st, Interval& intval_2nd, 
             bool loaded);

  void add_raw(const DynamicBuffer& cells);

  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& from_key);

  void expand(Interval& interval) const;

  void expand_begin(Interval& interval) const;

  void expand_end(Interval& interval) const;
  

  const std::string to_string(bool with_cells=false) const;
  
  private:

  void _allocate();

  void _move_fwd(uint32_t offset, int32_t by);

  void _move_bwd(uint32_t offset, int32_t by);

  void _remove(Cell* cell);

  uint32_t _narrow(const DB::Cell::Key& key) const;

  void _remove_overhead(uint32_t offset, const DB::Cell::Key& key, 
                        uint32_t revs);

  Cell**            m_cells;
  uint32_t          m_cap;
  uint32_t          m_size;
  uint32_t          m_size_bytes;
  uint32_t          m_max_revs;
  uint64_t          m_ttl;

};

*/

}}}

/*
#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif 
*/
#endif // swcdb_db_Cells_Mutable_h