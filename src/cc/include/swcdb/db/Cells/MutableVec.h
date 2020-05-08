/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_MutableVec_h
#define swcdb_db_cells_MutableVec_h


#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {


class MutableVec : private std::vector<Mutable*> {
  
  typedef std::vector<Mutable*> Vec;
  using Vec::vector;

  public:

  using Vec::begin;
  using Vec::end;
  
  const Types::KeySeq key_seq;
  uint32_t            split_size;
  uint32_t            max_revs;
  uint64_t            ttl;
  Types::Column       type;

  explicit MutableVec(const Types::KeySeq key_seq, uint32_t split_size=100000, 
                      const uint32_t max_revs=1, const uint64_t ttl_ns=0, 
                      const Types::Column type=Types::Column::PLAIN) 
                      : key_seq(key_seq), split_size(split_size), 
                        max_revs(max_revs), ttl(ttl_ns), type(type) {
  }

  ~MutableVec() {
    for(auto cells : *this)
      delete cells;
  }

  void free() {
    for(auto cells : *this)
      delete cells;
    clear();
  }

  void configure(uint32_t split,
                 const uint32_t revs=1, const uint64_t ttl_ns=0, 
                 const Types::Column typ=Types::Column::PLAIN) {
    split_size = split;
    max_revs = revs;
    ttl = ttl_ns;
    type = typ;
  }

  MutableVec operator=(const MutableVec &other) = delete;
  
  bool empty() const {
    return Vec::empty();
  }
  
  size_t size() const {
    size_t sz = 0;
    for(auto it = begin(); it < end(); ++it)
      sz += (*it)->size();
    return sz;
  }
  
  size_t size_bytes() const {
    size_t sz = 0;
    for(auto it = begin(); it < end(); ++it)
      sz += (*it)->size_bytes();
    return sz;
  }

  bool split(Mutable& cells, iterator it) {
    if(cells.size() >= split_size) {
      cells.split(**insert(it, new Mutable(key_seq, max_revs, ttl, type)));
      return true;
    }
    return false;
  }

  void add_sorted(const Cell& cell) {
    if(Vec::empty())
      push_back(new Mutable(key_seq, max_revs, ttl, type));
    back()->add_sorted(cell);
    split(*back(), end());
  }

  void add_raw(const Cell& cell) {
    if(Vec::empty())
      return add_sorted(cell);
    
    Mutable* cells;
    for(auto it = begin(); it < end();) {
      cells = *it;
      if(++it == end() || 
         DB::KeySeq::compare(key_seq, cell.key, (*it)->front()->key) 
                                                    == Condition::GT) {
        cells->add_raw(cell);
        split(*cells, it);
        return;
      }
    }
  }

  void add_raw(const Cell& cell, size_t* offset_itp, size_t* offsetp) {
    if(Vec::empty())
      return add_sorted(cell);
    
    Mutable* cells;
    for(auto it = begin()+*offset_itp; it < end(); ++*offset_itp, *offsetp=0) {
      cells = *it;
      if(++it == end() || 
         DB::KeySeq::compare(key_seq, cell.key, (*it)->front()->key) 
                                                    == Condition::GT) {
        cells->add_raw(cell, offsetp);
        if(split(*cells, it))
          *offsetp = 0;
        return;
      }
    }
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells) {         
    for(auto it = begin(); it < end() && 
                           (!threshold || threshold > cells.fill()) && 
                           (!max_cells || max_cells > cell_count);) {
      (*it)->write_and_free(cells, cell_count, intval, threshold, max_cells);
      if((*it)->empty()) {
        delete *it;
        erase(it);
      }
    }
  }

};

}}}
/*
#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/MutableVec.cc"
#endif 
*/
#endif // swcdb_db_Cells_MutableVec_h