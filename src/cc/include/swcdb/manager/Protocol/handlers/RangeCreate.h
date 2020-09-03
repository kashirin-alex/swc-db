/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_RangeCreate_h
#define swc_manager_Protocol_handlers_RangeCreate_h

#include "swcdb/db/Protocol/Mngr/params/RangeCreate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void range_create(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::RangeCreateRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeCreateReq params;
    params.decode(&ptr, &remain);
    std::cout << "RangeCreate: " << params.to_string() << "\n";

    auto col = Env::Mngr::mngd_columns()->get_column(
      rsp_params.err, params.cid);
    if(rsp_params.err) {
      if(rsp_params.err == Error::COLUMN_MARKED_REMOVED ||
         rsp_params.err == Error::MNGR_NOT_ACTIVE ||
         rsp_params.err == Error::COLUMN_NOT_EXISTS)
        goto send_response;
    }

    auto range = col->create_new_range(params.rgrid);
    if(range && range->rid) {
      rsp_params.rid = range->rid;
      rsp_params.err = Error::OK;
    } else {
      rsp_params.err = Error::RANGE_NOT_FOUND;
    }

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  
  send_response:
    try {
      std::cout << "RangeCreate(RSP): " << rsp_params.to_string() << "\n";
      auto cbp = CommBuf::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
}


}}}}

#endif // swc_manager_Protocol_handlers_RangeCreate_h