/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/MutableVec.h"


namespace SWC { namespace DB { namespace Cells {




SWC_SHOULD_INLINE
void MutableVec::configure(uint32_t split,
                           const uint32_t revs, const uint64_t ttl_ns,
                           const Types::Column typ) noexcept {
  split_size = split;
  max_revs = revs;
  ttl = ttl_ns;
  type = typ;
  for(auto cells : *this)
    cells->configure(max_revs, ttl, type);
}

void MutableVec::add_sorted(const Cell& cell) {
  if(Vec::empty())
    push_back(new Mutable(key_seq, max_revs, ttl, type));
  back()->add_sorted(cell);
  split(*back(), cend());
}

void MutableVec::add_raw(const Cell& cell) {
  for(auto it = cbegin(); it != cend();) {
    Mutable* cells = *it;
    if(++it == cend() ||
       DB::KeySeq::compare(key_seq, cell.key, (*it)->front().key)
                                                  == Condition::GT) {
      cells->add_raw(cell);
      split(*cells, it);
      return;
    }
  }
  return add_sorted(cell);
}

void MutableVec::add_raw(const Cell& cell,
                         size_t* offset_itp, size_t* offsetp) {
  if(*offset_itp >= Vec::size())
    *offset_itp = 0;

  for(auto it=cbegin()+*offset_itp; it != cend(); ++*offset_itp, *offsetp=0) {
     Mutable* cells = *it;
    if(++it == cend() ||
       DB::KeySeq::compare(key_seq, cell.key, (*it)->front().key)
                                                  == Condition::GT) {
      cells->add_raw(cell, offsetp);
      if(split(*cells, it))
        *offsetp = 0;
      return;
    }
  }
  return add_sorted(cell);
}

void MutableVec::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                                Interval& intval, uint32_t threshold,
                                uint32_t max_cells) {
  for(auto it = cbegin(); it != cend() &&
                         (!threshold || threshold > cells.fill()) &&
                         (!max_cells || max_cells > cell_count);) {
    (*it)->write_and_free(cells, cell_count, intval, threshold, max_cells);
    if((*it)->empty()) {
      delete *it;
      erase(it);
    }
  }
}



}}}
