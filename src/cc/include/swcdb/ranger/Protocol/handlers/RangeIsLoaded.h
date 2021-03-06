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
  Params::RangeIsLoadedRsp rsp_params(Error::OK);
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeIsLoadedReq params;
    params.decode(&ptr, &remain);

    auto range = Env::Rgr::columns()->get_range(
      rsp_params.err, params.cid, params.rid);
    if(range && range->is_loaded()) {
      if(range->can_be_merged())
        rsp_params.can_be_merged();

    } else if(!rsp_params.err) {
      rsp_params.err = Error::RGR_NOT_LOADED_RANGE;
    }

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }

  conn->send_response(Buffers::make(ev, rsp_params));
}


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeIsLoaded_h
