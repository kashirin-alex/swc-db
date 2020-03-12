/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_Protocol_handlers_ColumnCompact_h
#define swc_ranger_Protocol_handlers_ColumnCompact_h

#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void column_compact(ConnHandlerPtr conn, Event::Ptr ev) {
  Params::ColumnCompactRsp rsp_params;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    auto col = RangerEnv::columns()->get_column(rsp_params.err, params.cid);
    if(col != nullptr)
      col->compact();

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }
  
  send_response:
    try {
      auto cbp = CommBuf::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_ColumnCompact_h