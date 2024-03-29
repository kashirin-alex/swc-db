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

  ~MutableVec() noexcept {
    for(auto cells : *this)
      delete cells;
  }

  SWC_CAN_INLINE
  void free() noexcept {
    for(auto cells : *this)
      delete cells;
    clear();
  }

  void configure(uint32_t split,
                 const uint32_t revs, const uint64_t ttl_ns,
                 const Types::Column typ, bool finalized) noexcept;

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

  void add_raw(const Cell& cell, bool finalized);

  void add_raw(const Cell& cell,
               size_t* offset_itp, size_t* offsetp, bool finalized);

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold,
                      uint32_t max_cells);

  SWC_CAN_INLINE
  void check_sequence(const char* msg, bool w_assert=true) const {
    for(auto it = cbegin(); it != cend();) {
      Mutable* cells = *it;
      if(++it != cend() && DB::KeySeq::compare(key_seq, (*it)->front().key, cells->back().key) == Condition::GT) {
        SWC_LOG_OUT(
          LOG_ERROR,
          SWC_LOG_OSTREAM << "BAD cells-sequence at " << msg;
          cells->back().key.print(SWC_LOG_OSTREAM << "\n current-");
          (*it)->front().key.print(SWC_LOG_OSTREAM << "\n next-");
        );
        SWC_ASSERT(!w_assert);
      }
      cells->check_sequence(msg, w_assert);
    }
  }

  private:

  SWC_CAN_INLINE
  bool split(Mutable& cells, const const_iterator& it) {
    if(cells.size() >= split_size && !cells.has_one_key()) {
      Mutable next_cells(key_seq, max_revs, ttl, type);
      if(cells.split(next_cells, split_size, 0, true)) {
        insert(it, new Mutable(std::move(next_cells)));
        return true;
      }
    }
    return false;
  }

};


}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/MutableVec.cc"
#endif


#endif // swcdb_db_Cells_MutableVec_h
