/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_Report_h
#define swc_app_ranger_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void report(ConnHandlerPtr conn, Event::Ptr ev) {
  Protocol::Rgr::Params::ReportRsp rsp_params(Error::OK);

  try {
    Params::ReportReq params;
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    if(params.flags & Params::ReportReq::RANGES) {
      server::Rgr::Column::Ptr col;
      server::Rgr::Range::Ptr range;
      auto columns = RangerEnv::columns();
      for(size_t cidx = 0; (col=columns->get_next(cidx)) != nullptr; cidx++) {
        auto c = new Protocol::Rgr::Params::ReportRsp::Column();
        rsp_params.columns.push_back(c);
        c->cid = col->cfg.cid;
        for(size_t ridx = 0; (range=col->get_next(ridx)) != nullptr; ridx++) {
          auto r = new Protocol::Rgr::Params::ReportRsp::Range();
          c->ranges.push_back(r);
          r->rid = range->rid;
          range->get_interval(r->interval);
        }
      }

    }

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }

  try{
    auto cbp = CommBuf::make(rsp_params);
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_app_ranger_handlers_Report_h