/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/MutableVec.h"


namespace SWC { namespace DB { namespace Cells {


SWC_SHOULD_INLINE
MutableVec::MutableVec(const Types::KeySeq key_seq, uint32_t split_size,
                       const uint32_t max_revs, const uint64_t ttl_ns, 
                       const Types::Column type)
                      : key_seq(key_seq), split_size(split_size), 
                        max_revs(max_revs), ttl(ttl_ns), type(type) {
}

MutableVec::~MutableVec() {
  for(auto cells : *this)
    delete cells;
}

void MutableVec::free() {
  for(auto cells : *this)
    delete cells;
  clear();
}

SWC_SHOULD_INLINE
void MutableVec::configure(uint32_t split,
                           const uint32_t revs, const uint64_t ttl_ns,
                           const Types::Column typ) {
  split_size = split;
  max_revs = revs;
  ttl = ttl_ns;
  type = typ;
}

SWC_SHOULD_INLINE
bool MutableVec::empty() const {
  return Vec::empty();
}
  
size_t MutableVec::size() const {
  size_t sz = 0;
  for(auto it = begin(); it < end(); ++it)
    sz += (*it)->size();
  return sz;
}

size_t MutableVec::size_bytes() const {
  size_t sz = 0;
  for(auto it = begin(); it < end(); ++it)
    sz += (*it)->size_bytes();
  return sz;
}

size_t MutableVec::size_of_internal() const {
  size_t sz = 0;
  for(auto it = begin(); it < end(); ++it) {
    sz += (*it)->size_of_internal();
    sz += sizeof(*it);
  }
  return sz;
}

bool MutableVec::split(Mutable& cells, MutableVec::iterator it) {
  if(cells.size() >= split_size) {
    cells.split(**insert(it, new Mutable(key_seq, max_revs, ttl, type)));
    return true;
  }
  return false;
}

void MutableVec::add_sorted(const Cell& cell) {
  if(Vec::empty())
    push_back(new Mutable(key_seq, max_revs, ttl, type));
  back()->add_sorted(cell);
  split(*back(), end());
}

void MutableVec::add_raw(const Cell& cell) {
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

void MutableVec::add_raw(const Cell& cell, 
                         size_t* offset_itp, size_t* offsetp) {
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

void MutableVec::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
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



}}}
