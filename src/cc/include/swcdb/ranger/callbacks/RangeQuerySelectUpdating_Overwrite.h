/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectUpdating_Overwrite_h
#define swcdb_ranger_callbacks_RangeQuerySelectUpdating_Overwrite_h


#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectUpdating_Overwrite final
    : public RangeQuerySelectUpdating {
  public:

  typedef std::shared_ptr<RangeQuerySelectUpdating_Overwrite> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectUpdating_Overwrite(const Comm::ConnHandlerPtr& conn,
                                     const Comm::Event::Ptr& ev,
                                     DB::Specs::Interval&& req_spec,
                                     const RangePtr& a_range)
                                    : RangeQuerySelectUpdating(
                                        conn,
                                        ev,
                                        std::move(req_spec),
                                        a_range
                                      ) {
    only_keys = false;
  }

  virtual ~RangeQuerySelectUpdating_Overwrite() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    uint8_t* value_was = cell.value;
    uint32_t vlen_was = cell.vlen;
    uint32_t pos = spec.updating->operation.get_pos();
    if(pos >= vlen_was) {
      pos = vlen_was;
      cell.vlen = vlen_was + spec.updating->vlen;
    } else {
      cell.vlen = spec.updating->vlen < vlen_was - pos
        ? vlen_was
        : spec.updating->vlen + pos;
    }
    cell.own = true;
    cell.value = static_cast<uint8_t*>(
      memcpy(
        new uint8_t[cell.vlen],
        value_was,
        pos
      )
    );
    memcpy(
      cell.value + pos,
      spec.updating->value,
      spec.updating->vlen
    );
    uint32_t at = pos + spec.updating->vlen;
    if(at < vlen_was) {
      memcpy(
        cell.value + at,
        value_was + at,
        vlen_was - at
      );
    }
  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Overwrite_h
