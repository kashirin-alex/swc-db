/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectUpdating_Append_h
#define swcdb_ranger_callbacks_RangeQuerySelectUpdating_Append_h


#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectUpdating_Append final
    : public RangeQuerySelectUpdating {
  public:

  typedef std::shared_ptr<RangeQuerySelectUpdating_Append> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectUpdating_Append(const Comm::ConnHandlerPtr& conn,
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

  virtual ~RangeQuerySelectUpdating_Append() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    cell.value = static_cast<uint8_t*>(memcpy(
      new uint8_t[cell.vlen + spec.updating->vlen],
      cell.value,
      cell.vlen
    ));
    memcpy(
      cell.value + cell.vlen,
      spec.updating->value,
      spec.updating->vlen
    );
    cell.vlen += spec.updating->vlen;
    cell.own = true;
  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Append_h
