/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_Protocol_handlers_RangeLocate_h
#define swc_ranger_Protocol_handlers_RangeLocate_h

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/ranger/callbacks/RangeLocateScanCommit.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_locate(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Params::RangeLocateReq params;
  Ranger::RangePtr range;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    range = RangerEnv::columns()->get_range(err, params.cid, params.rid);
    
    if(!err && (range == nullptr || !range->is_loaded()))
      err = Error::RS_NOT_LOADED_RANGE;

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }
  
  try{

    if(err) {
      Protocol::Rgr::Params::RangeLocateRsp rsp_params(err);
      
      SWC_LOG_OUT(LOG_DEBUG) 
        << rsp_params.to_string() << " " << params.to_string() 
        << SWC_LOG_OUT_END;

      auto cbp = CommBuf::make(rsp_params);
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);

      return;
    }

    Ranger::ReqScan::Ptr req;
    if(params.flags & Protocol::Rgr::Params::RangeLocateReq::COMMIT) {
      req = std::make_shared<Ranger::Callback::RangeLocateScanCommit>(
        conn, ev,
        params.range_begin, //params.range_end,
        range,
        params.flags
      );
    } else {
      req = std::make_shared<Ranger::Callback::RangeLocateScan>(
        conn, ev,
        params.range_begin, params.range_end,
        range,
        params.flags
      );
      if(params.flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE)
        req->spec.range_offset.copy(params.range_offset);
    }
    range->scan(req);
    
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_RangeLocate_h