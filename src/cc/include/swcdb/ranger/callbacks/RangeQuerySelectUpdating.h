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

  bool has_update() const noexcept override {
    return true;
  }

  void update(DB::Cells::Mutable& blk_cells) override {
    if(cells.empty())
      return;
      
    const bool auto_ts(spec.updating->timestamp == DB::Cells::TIMESTAMP_AUTO);
    const bool set_ts(spec.updating->timestamp != DB::Cells::TIMESTAMP_NULL);

    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();

    size_t log_offset_it_hint = 0;
    size_t log_offset_hint = 0;
    size_t blk_offset_hint = 0;

    auto& commitlog = range->blocks.commitlog;
    {
      Core::ScopedLock commitlog_lock(commitlog.cells_mutex());

      if(expired(remain/100000))
        throw Error::Exception(Error::REQUEST_TIMEOUT, "");

      for(DB::Cells::Cell updated_cell; remain; ) {
        updated_cell.read(&ptr, &remain);

        updated_cell.value = spec.updating->value;
        updated_cell.vlen = spec.updating->vlen;

        auto ts = Time::now_ns();
        if(auto_ts) {
          updated_cell.set_timestamp_with_rev_is_ts(ts);
        } else {
          if(set_ts)
            updated_cell.set_timestamp(spec.updating->timestamp);
          updated_cell.set_revision(ts);
        }

        commitlog._add(updated_cell, &log_offset_it_hint, &log_offset_hint);
        blk_cells.add_raw(updated_cell, &blk_offset_hint);
      }
    }
    commitlog.commit();

  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_h
