/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_Report_h
#define swc_manager_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Mngr/params/Report.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void report(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  
  int err = Error::OK;
  CommBuf::Ptr cbp;

  try {
    
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    auto func((Params::Report::Function)Serialization::decode_i8(&ptr, &remain));
    
    switch(func) {

      case Params::Report::Function::CLUSTER_STATUS: {
        uint8_t status = 0;
        //all ready ? Env::Mngr::mngd_columns()->is_active(rsp_params.err, params.cid); 
        cbp = CommBuf::make(Params::Report::RspClusterStatus(status), 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMN_STATUS: {
        Params::Report::ReqColumnStatus params;
        params.decode(&ptr, &remain);
        
        Env::Mngr::mngd_columns()->is_active(err, params.cid); 
        if(err)
          goto send_error;
          
        Params::Report::RspColumnStatus rsp_params;
        auto col = Env::Mngr::columns()->get_column(err, params.cid);
        if(!err) {
          col->state(err);
          if(err == Error::COLUMN_NOT_READY) {
            err = Error::OK;
            rsp_params.status = Error::COLUMN_NOT_READY;
          }
        }
        if(err)
          goto send_error;
        std::vector<Manager::Range::Ptr> ranges;
        col->get_ranges(ranges);
        rsp_params.ranges.resize(ranges.size());
        size_t i = 0; 
        for(auto& r : ranges) {
          auto& set_r = rsp_params.ranges[i];
          set_r.status = r->state();
          set_r.rid = r->rid;
          set_r.rgr_id = r->get_rgr_id();
          ++i;
        }
        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      default:
        err = Error::NOT_IMPLEMENTED;
        break;
    }
    
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  
  send_error:
    cbp = CommBuf::make(4);
    cbp->append_i32(err);


  send_response:
    try {
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }

}



}}}}

#endif // swc_manager_Protocol_handlers_:Report_h
