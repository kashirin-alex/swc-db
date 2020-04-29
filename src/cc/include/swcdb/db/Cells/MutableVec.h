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
  uint32_t            split_size;
  const Types::KeySeq key_seq;
  Types::Column       type;
  uint32_t            max_revs;
  uint64_t            ttl;

  explicit MutableVec(uint32_t split_size, const Types::KeySeq key_seq,
                      const uint32_t max_revs=1, const uint64_t ttl_ns=0, 
                      const Types::Column type=Types::Column::PLAIN) 
                      : split_size(split_size), 
                        key_seq(key_seq), type(type), 
                        max_revs(max_revs), ttl(ttl_ns) {
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
    max_revs = revs;
    ttl = ttl_ns;
    type = typ;
    split_size = split;
  }

  MutableVec operator=(const MutableVec &other) = delete;
  
  bool empty() const {
    return Vec::empty() || front()->empty();
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

  void add_sorted(const Cell& cell) {
    if(Vec::empty())
      push_back(new Mutable(key_seq, max_revs, ttl, type));
    back()->add_sorted(cell);
  }

  void add_raw(const Cell& cell) {
    if(Vec::empty())
      return add_sorted(cell);
    
    Mutable* cells;
    for(auto it = begin(); it < end();) {
      cells = *it;
      if(++it == end() || 
         DB::KeySeq::compare(key_seq, cells->back()->key, cell.key) 
                                                    != Condition::GT) {
        cells->add_raw(cell);
        if(cells->size() >= split_size * 2) {
          push_back(new Mutable(key_seq, max_revs, ttl, type));
          cells->split(*back());
        }
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