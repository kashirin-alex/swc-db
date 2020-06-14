/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_RangeCreate_h
#define swc_manager_Protocol_handlers_RangeCreate_h

#include "swcdb/db/Protocol/Mngr/params/RangeCreate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void range_create(ConnHandlerPtr conn, Event::Ptr ev) {
  Params::RangeCreateRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeCreateReq params;
    params.decode(&ptr, &remain);
    std::cout << "RangeCreate: " << params.to_string() << "\n";

    Env::Mngr::mngd_columns()->is_active(rsp_params.err, params.cid); 
    if(rsp_params.err)
      goto send_response;

    auto col = Env::Mngr::columns()->get_column(rsp_params.err, params.cid);
    if(rsp_params.err)
      goto send_response;

    col->state(rsp_params.err);
    if(rsp_params.err && rsp_params.err == Error::COLUMN_MARKED_REMOVED)
      goto send_response;

    auto range = col->create_new_range(params.rgrid);
    rsp_params.rid = range->rid;

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }
  
  send_response:
    try {
      std::cout << "RangeCreate(RSP): " << rsp_params.to_string() << "\n";
      auto cbp = CommBuf::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
}


}}}}

#endif // swc_manager_Protocol_handlers_RangeCreate_h