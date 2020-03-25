/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Cells_Vector_h
#define swcdb_db_Cells_Vector_h

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/Interval.h"
#include "swcdb/db/Cells/VectorBig.h"

namespace SWC { namespace DB { namespace Cells {

typedef VectorBig Vector;

/*
class Vector : private std::vector<Cell*> {
  public:

  typedef std::shared_ptr<Vector>                  Ptr;
  typedef std::function<bool(const Cell&, bool&)>  Selector_t;

  static const uint8_t  _cell_sz = sizeof(Cell*);

  using std::vector<Cell*>::vector;
  using std::vector<Cell*>::empty;
  using std::vector<Cell*>::capacity;
  using std::vector<Cell*>::reserve;
  using std::vector<Cell*>::size;
  using std::vector<Cell*>::back;
  using std::vector<Cell*>::front;
  using std::vector<Cell*>::begin;
  using std::vector<Cell*>::end;
  using std::vector<Cell*>::operator[];

  size_t            bytes;
  Types::Column     type;
  uint32_t          max_revs;
  uint64_t          ttl;

  static Ptr make(const uint32_t max_revs=1, 
                  const uint64_t ttl_ns=0, 
                  const Types::Column type=Types::Column::PLAIN);

  explicit Vector(const uint32_t max_revs=1, const uint64_t ttl_ns=0, 
                  const Types::Column type=Types::Column::PLAIN);

  explicit Vector(Vector& other);

  Vector& operator=(const Vector& other) = delete;

  virtual ~Vector();

  void free();

  void reset(const uint32_t revs=1, const uint64_t ttl_ns=0, 
             const Types::Column typ=Types::Column::PLAIN);

  void configure(const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN);

  const size_t size_bytes() const;

  void take_sorted(Vector& other);


  void add_sorted(const Cell& cell, bool no_value=false);
  
  void add_sorted_no_cpy(Cell* cell);
  
  const size_t add_sorted(const uint8_t* ptr, size_t remain);


  void add_raw(const DynamicBuffer& cells);
  
  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& from_key);
  
  void add_raw(const Cell& e_cell);
  

  Cell* takeout_begin(size_t idx);

  Cell* takeout_end(size_t idx);


  void write(DynamicBuffer& cells) const;

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells);

  bool write_and_free(const DB::Cell::Key& key_start, 
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);


  const std::string to_string(bool with_cells=false) const;
  

  const bool has_one_key() const;

  void get(int32_t idx, DB::Cell::Key& key) const;
  
  const bool get(const DB::Cell::Key& key, Condition::Comp comp, 
                 DB::Cell::Key& res) const;


  void scan(const Specs::Interval& specs, Vector& cells, 
            size_t& cell_offset, 
            const std::function<bool()>& reached_limits, 
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

  void scan(Interval& interval, Vector& cells) const;


  void expand(Interval& interval) const;

  void expand_begin(Interval& interval) const;

  void expand_end(Interval& interval) const;
  

  void split(size_t from, Vector& cells, 
             Interval& intval_1st, Interval& intval_2nd, bool loaded);

  private:

  void _add_remove(const Cell& e_cell, size_t offset);

  void _add_plain(const Cell& e_cell, size_t offset);

  void _add_counter(const Cell& e_cell, size_t offset);
  

  static const uint8_t narrow_sz = 20;

  size_t _narrow(const DB::Cell::Key& key) const;

  void _push_back(Cell* cell);

  void _insert(iterator it, const Cell& cell);

  void _remove(iterator it);

  void _remove(iterator it_begin, iterator it_end);
  
  void _remove_overhead(iterator it, const DB::Cell::Key& key, uint32_t revs);

};
*/

}}}

/*
#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Vector.cc"
#endif 
*/

#endif // swcdb_db_Cells_Vector_h