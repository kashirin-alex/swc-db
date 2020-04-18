/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_RgrGet_h
#define swc_manager_Protocol_handlers_RgrGet_h

#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void rgr_get(ConnHandlerPtr conn, Event::Ptr ev) {
  Params::RgrGetReq params;
  Params::RgrGetRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    Env::Mngr::mngd_columns()->is_active(rsp_params.err, params.cid); 
    if(rsp_params.err != Error::OK)
      goto send_response;

    auto col = Env::Mngr::columns()->get_column(rsp_params.err, params.cid);
    if(rsp_params.err != Error::OK)
      goto send_response;

    col->state(rsp_params.err);
    if(rsp_params.err != Error::OK)
      goto send_response;
    
    Manager::Range::Ptr range;
    if(!params.rid) {
      range = col->get_range(
        rsp_params.err, 
        params.range_begin, 
        params.range_end,
        params.next_range
      );
      if(range != nullptr) {
        range->get_interval(rsp_params.range_begin, rsp_params.range_end);
        rsp_params.range_begin.remove(0);
        rsp_params.range_begin.remove(0);
        rsp_params.range_end.remove(0);
        rsp_params.range_end.remove(0);
      }
    } else 
      range = col->get_range(rsp_params.err, params.rid);
      
    if(range == nullptr) {
      rsp_params.err = Error::RANGE_NOT_FOUND;
      goto send_response;
    }

    Env::Mngr::rangers()->rgr_get(
      range->get_rgr_id(), rsp_params.endpoints);
    if(rsp_params.endpoints.empty()) {
      rsp_params.err = Error::RS_NOT_READY;
      goto send_response;
    }

    rsp_params.cid = range->cfg->cid;
    rsp_params.rid = range->rid;

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }
  
  send_response:

    SWC_LOG_OUT(LOG_DEBUG) 
      << params.to_string() << " " << rsp_params.to_string() 
      << SWC_LOG_OUT_END;

    try {
      auto cbp = CommBuf::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
}

  
// flag(if cid==1) 
//      in(cid+interval)  out(cid + rid + rgr-endpoints + ?next) 
// else 
//      in(cid+rid)        out(cid + rid + rgr-endpoints)

}}}}

#endif // swc_manager_Protocol_handlers_RgrGet_h