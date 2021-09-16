/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_MutableVec_h
#define swcdb_db_cells_MutableVec_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {



class MutableVec final : private Core::Vector<Mutable*> {

  typedef Core::Vector<Mutable*> Vec;

  public:

  using Vec::begin;
  using Vec::end;

  const Types::KeySeq key_seq;
  uint32_t            split_size;
  uint32_t            max_revs;
  uint64_t            ttl;
  Types::Column       type;

  SWC_CAN_INLINE
  explicit MutableVec(const Types::KeySeq a_key_seq, uint32_t a_split_size=100000,
                      const uint32_t a_max_revs=1, const uint64_t ttl_ns=0,
                      const Types::Column a_type=Types::Column::PLAIN) noexcept
                      : key_seq(a_key_seq), split_size(a_split_size),
                        max_revs(a_max_revs), ttl(ttl_ns), type(a_type) {
  }

  MutableVec(const MutableVec&) = delete;

  MutableVec(const MutableVec&&) = delete;

  MutableVec& operator=(const MutableVec&) = delete;

  ~MutableVec();

  void free();

  void configure(uint32_t split,
                 const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN) noexcept;

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return Vec::empty();
  }

  SWC_CAN_INLINE
  size_t size() const noexcept {
    size_t sz = 0;
    for(auto cells : *this)
      sz += cells->size();
    return sz;
  }

  SWC_CAN_INLINE
  size_t size_bytes() const noexcept {
    size_t sz = 0;
    for(auto cells : *this)
      sz += cells->size_bytes();
    return sz;
  }

  SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    size_t sz = 0;
    for(auto cells : *this) {
      sz += cells->size_of_internal();
      sz += sizeof(cells);
    }
    return sz;
  }

  void add_sorted(const Cell& cell);

  void add_raw(const Cell& cell);

  void add_raw(const Cell& cell, size_t* offset_itp, size_t* offsetp);

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold,
                      uint32_t max_cells);

  private:

  bool split(Mutable& cells, const_iterator it);

};



SWC_CAN_INLINE
MutableVec::~MutableVec() {
  for(auto cells : *this)
    delete cells;
}

SWC_CAN_INLINE
void MutableVec::free() {
  for(auto cells : *this)
    delete cells;
  clear();
}

SWC_CAN_INLINE
bool MutableVec::split(Mutable& cells, MutableVec::const_iterator it) {
  if(cells.size() >= split_size && cells.can_split()) {
    cells.split(**insert(it, new Mutable(key_seq, max_revs, ttl, type)));
    return true;
  }
  return false;
}

SWC_CAN_INLINE
void MutableVec::add_sorted(const Cell& cell) {
  if(Vec::empty())
    push_back(new Mutable(key_seq, max_revs, ttl, type));
  back()->add_sorted(cell);
  split(*back(), cend());
}



}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/MutableVec.cc"
#endif


#endif // swcdb_db_Cells_MutableVec_h
