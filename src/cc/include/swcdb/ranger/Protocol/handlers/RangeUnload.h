/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeUnload_h
#define swcdb_ranger_Protocol_handlers_RangeUnload_h

#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_unload(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::RangeUnloadRsp rsp_params(Error::OK);
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeUnload params;
    params.decode(&ptr, &remain);

    auto col = Env::Rgr::columns()->get_column(params.cid);
    if(col) {
      col->add_managing(Ranger::Callback::RangeUnload::Ptr(
        new Ranger::Callback::RangeUnload(
          conn, ev, params.cid, params.rid, true)
      ));
      return;
    }
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  conn->send_response(Buffers::make(ev, rsp_params));
}


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeUnload_h
