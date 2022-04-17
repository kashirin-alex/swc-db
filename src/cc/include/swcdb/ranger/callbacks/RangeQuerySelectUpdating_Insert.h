/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectUpdating_Insert_h
#define swcdb_ranger_callbacks_RangeQuerySelectUpdating_Insert_h


#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectUpdating_Insert final
    : public RangeQuerySelectUpdating {
  public:

  typedef std::shared_ptr<RangeQuerySelectUpdating_Insert> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectUpdating_Insert(const Comm::ConnHandlerPtr& conn,
                                   const Comm::Event::Ptr& ev,
                                   DB::Specs::Interval&& req_spec,
                                   const RangePtr& a_range)
                                  : RangeQuerySelectUpdating(
                                      conn,
                                      ev,
                                      std::move(req_spec),
                                      a_range
                                    ) {
  }

  virtual ~RangeQuerySelectUpdating_Insert() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    uint32_t pos = spec.updating->operation.get_pos();
    if(pos > cell.vlen)
      pos = cell.vlen;
    uint8_t* value_was = cell.value;
    cell.value = static_cast<uint8_t*>(memcpy(
      new uint8_t[cell.vlen + spec.updating->vlen],
      value_was,
      pos
    ));
    memcpy(
      cell.value + pos,
      spec.updating->value,
      spec.updating->vlen
    );
    memcpy(
      cell.value + pos + spec.updating->vlen,
      value_was + pos,
      cell.vlen - pos
    );
    cell.vlen += spec.updating->vlen;
    cell.own = true;
  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Insert_h
