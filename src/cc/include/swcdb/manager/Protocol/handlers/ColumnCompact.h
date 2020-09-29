/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_ColumnCompact_h
#define swc_manager_Protocol_handlers_ColumnCompact_h

#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void column_compact(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {
  Params::ColumnCompactRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    Env::Mngr::mngd_columns()->get_column(rsp_params.err, params.cid);
    if(rsp_params.err)
      goto send_response;

    Env::Mngr::rangers()->column_compact(rsp_params.err, params.cid);

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  
  send_response:
    try {
      auto cbp = Comm::Buffers::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
}


}}}}

#endif // swc_manager_Protocol_handlers_ColumnCompact_h