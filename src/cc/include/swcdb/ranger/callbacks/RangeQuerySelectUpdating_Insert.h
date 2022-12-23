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
    only_keys = false;
  }

  virtual ~RangeQuerySelectUpdating_Insert() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    StaticBuffer v;
    DB::Types::Encoder encoder = cell.get_value(v, false);
    uint32_t sz = v.size + spec.updating->vlen;
  
    uint32_t pos = spec.updating->operation.get_pos();
    if(pos > v.size)
      pos = v.size;
    uint8_t* ptr = static_cast<uint8_t*>(memcpy(
      new uint8_t[sz],
      v.base,
      pos
    ));
    memcpy(
      ptr + pos,
      spec.updating->value,
      spec.updating->vlen
    );
    memcpy(
      ptr + pos + spec.updating->vlen,
      v.base + pos,
      v.size - pos
    );

    if(encoder == DB::Types::Encoder::DEFAULT) {
      cell.own = true;
      cell.value = ptr;
      cell.vlen = sz;
    } else {
      cell.set_value(encoder, ptr, sz);
      delete [] ptr;
    }

  }

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Insert_h
