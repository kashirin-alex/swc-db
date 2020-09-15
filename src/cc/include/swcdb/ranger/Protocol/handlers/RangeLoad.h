/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_Protocol_handlers_RangeLoad_h
#define swc_ranger_Protocol_handlers_RangeLoad_h

#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"
#include "swcdb/ranger/callbacks/RangeLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_load(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeLoad params;
    params.decode(&ptr, &remain);

    int err = Error::OK;
    RangerEnv::columns()->load_range(
      err,
      params.cid, params.rid, *params.schema.get(),
      std::make_shared<Ranger::Callback::RangeLoaded>(
        conn, ev, params.cid, params.rid)
    );
     
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_RangeLoad_h