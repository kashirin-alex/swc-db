/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_Protocol_handlers_RangeLocate_h
#define swc_ranger_Protocol_handlers_RangeLocate_h

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/ranger/callbacks/RangeLocateScanCommit.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_locate(ConnHandlerPtr conn, Event::Ptr ev) {
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

    DB::Cells::Result cells(
      range->cfg->cell_versions(), 
      range->cfg->cell_ttl(), 
      range->cfg->column_type()
    );

    Ranger::ReqScan::Ptr req;
    if(params.flags & Protocol::Rgr::Params::RangeLocateReq::COMMIT) {
      req = std::make_shared<Ranger::Callback::RangeLocateScanCommit>(
        conn, ev,
        DB::Specs::Interval(params.range_begin, params.range_end),
        cells,
        range,
        params.flags
      );
    } else {
      req = std::make_shared<Ranger::Callback::RangeLocateScan>(
        conn, ev,
        DB::Specs::Interval(params.range_begin, params.range_end),
        cells,
        range,
        params.flags
      );
      if(params.flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE)
        req->spec.range_offset.copy(params.range_offset);
    }

    req->spec.flags.limit = 1;
    range->scan(req);
    
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_RangeLocate_h