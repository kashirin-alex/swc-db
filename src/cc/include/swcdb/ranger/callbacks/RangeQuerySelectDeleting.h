/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectDeleting_h
#define swcdb_ranger_callbacks_RangeQuerySelectDeleting_h


#include "swcdb/ranger/callbacks/RangeQuerySelect.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectDeleting : public RangeQuerySelect {
  public:

  typedef std::shared_ptr<RangeQuerySelectDeleting> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectDeleting(const Comm::ConnHandlerPtr& conn,
                           const Comm::Event::Ptr& ev,
                           DB::Specs::Interval&& req_spec,
                           const RangePtr& a_range)
                          : RangeQuerySelect(
                              conn,
                              ev,
                              std::move(req_spec),
                              a_range
                            ),
                            del_flag(
                              a_range->cfg->cell_versions() == 1
                                ? DB::Cells::Flag::DELETE_LE
                                : DB::Cells::Flag::DELETE_EQ
                            ) {
  }

  virtual ~RangeQuerySelectDeleting() noexcept { }

  bool has_update() const noexcept override {
    return true;
  }

  void update(DB::Cells::Mutable& blk_cells) override {
    if(cells.ptr == cells.mark)
      return;

    const uint8_t* ptr = cells.mark;
    size_t remain = cells.fill() - (cells.mark - cells.base);

    size_t log_offset_it = 0;
    size_t log_offset_hint = 0;
    size_t blk_offset_hint = 0;

    auto& commitlog = range->blocks.commitlog;
    {
      Core::ScopedLock commitlog_lock(commitlog.cells_mutex());

      if(expired(remain/100000))
        throw Error::Exception(Error::REQUEST_TIMEOUT, "");

      for(DB::Cells::Cell updated_cell; remain; ) {
        updated_cell.read(&ptr, &remain, false);

        updated_cell.flag = del_flag;
        updated_cell.set_revision(Time::now_ns());

        commitlog._add(updated_cell, &log_offset_it, &log_offset_hint);
        blk_cells.add_raw(updated_cell, &blk_offset_hint, true);
      }
    }
    commitlog.commit();

    cells.set_mark();
  }

  private:
  const DB::Cells::Flag del_flag;

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectDeleting_h
