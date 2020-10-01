/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeIsLoaded_h
#define swcdb_ranger_Protocol_handlers_RangeIsLoaded_h

#include "swcdb/db/Protocol/Rgr/params/RangeIsLoaded.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_is_loaded(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeIsLoaded params;
    params.decode(&ptr, &remain);

    int err = Error::OK;
    auto range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);
      
    if(range != nullptr && range->is_loaded()) {
      conn->response_ok(ev);
    } else {
      if(err == Error::OK)
        err = Error::RGR_NOT_LOADED_RANGE;
      conn->send_error(err, "", ev);
    }
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeIsLoaded_h
