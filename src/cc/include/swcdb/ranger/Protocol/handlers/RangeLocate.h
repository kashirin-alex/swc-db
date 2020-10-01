/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeLocate_h
#define swcdb_ranger_Protocol_handlers_RangeLocate_h

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/ranger/callbacks/RangeLocateScanCommit.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_locate(const Comm::ConnHandlerPtr& conn, 
                  const Comm::Event::Ptr& ev) {
  int err = Error::OK;
  Params::RangeLocateReq params;
  Ranger::RangePtr range;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);
    
    if(!err && (range == nullptr || !range->is_loaded()))
      err = Error::RGR_NOT_LOADED_RANGE;

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  try{

    if(err) {
      Protocol::Rgr::Params::RangeLocateRsp rsp_params(err);
      
      SWC_LOG_OUT(LOG_DEBUG,
        params.print(SWC_LOG_OSTREAM);
        rsp_params.print(SWC_LOG_OSTREAM << ' ');
      );

      auto cbp = Comm::Buffers::make(rsp_params);
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
  
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  
}
  

}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeLocate_h