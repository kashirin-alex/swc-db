/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectUpdating_h
#define swcdb_ranger_callbacks_RangeQuerySelectUpdating_h


#include "swcdb/ranger/callbacks/RangeQuerySelect.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectUpdating : public RangeQuerySelect {
  public:

  typedef std::shared_ptr<RangeQuerySelectUpdating> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectUpdating(const Comm::ConnHandlerPtr& conn,
                           const Comm::Event::Ptr& ev,
                           DB::Specs::Interval&& req_spec,
                           const RangePtr& a_range)
                          : RangeQuerySelect(
                              conn, ev, std::move(req_spec), a_range) {
  }

  virtual ~RangeQuerySelectUpdating() noexcept { }

  void update(DB::Cells::Mutable& blk_cells) override {
    if(cells.empty())
      return;

    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();

    size_t log_offset_it_hint = 0;
    size_t log_offset_hint = 0;
    size_t blk_offset_hint = 0;
    auto& commitlog = range->blocks.commitlog;
    commitlog.cells_lock();

    for(DB::Cells::Cell updated_cell; remain; ) {
      updated_cell.read(&ptr, &remain);

      updated_cell.value = spec.updating->value;
      updated_cell.vlen = spec.updating->vlen;

      if(spec.updating->timestamp == DB::Cells::TIMESTAMP_AUTO) {
        updated_cell.set_timestamp(Time::now_ns());
        updated_cell.control |= DB::Cells::REV_IS_TS;
        if(updated_cell.control & DB::Cells::HAVE_REVISION)
          updated_cell.control ^= DB::Cells::HAVE_REVISION;
      } else {
        if(spec.updating->timestamp != DB::Cells::TIMESTAMP_NULL)
          updated_cell.set_timestamp(spec.updating->timestamp);
        updated_cell.set_revision(Time::now_ns());
        if(updated_cell.control & DB::Cells::REV_IS_TS)
          updated_cell.control ^= DB::Cells::REV_IS_TS;
      }

      commitlog._add(updated_cell, &log_offset_it_hint, &log_offset_hint);
      blk_cells.add_raw(updated_cell, &blk_offset_hint);
    }

    commitlog.cells_unlock();
  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_h
