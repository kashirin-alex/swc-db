/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_Protocol_handlers_Report_h
#define swc_ranger_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void report(ConnHandlerPtr conn, Event::Ptr ev) {
  Protocol::Rgr::Params::ReportRsp rsp_params(Error::OK);

  try {
    Params::ReportReq params;
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    auto rgr_data = RangerEnv::rgr_data();
    if(!(rsp_params.rgr_id = rgr_data->id)) {
      rsp_params.err = Error::RS_NOT_READY;
      goto send_response;
    }
    rsp_params.endpoints.assign(
      rgr_data->endpoints.begin(), rgr_data->endpoints.end());

    if(params.flags & Params::ReportReq::RANGES) {
      Ranger::Column::Ptr col;
      Ranger::RangePtr range;
      auto columns = RangerEnv::columns();
      for(size_t cidx = 0; (col=columns->get_next(cidx)) != nullptr; ++cidx) {
        auto c = new Protocol::Rgr::Params::ReportRsp::Column();
        rsp_params.columns.push_back(c);
        c->cid = col->cfg.cid;
        c->col_seq = col->cfg.sequence;
        for(size_t ridx = 0; (range=col->get_next(ridx)) != nullptr; ++ridx) {
          auto r = new Protocol::Rgr::Params::ReportRsp::Range(c->col_seq);
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

  send_response:
  try{
    auto cbp = CommBuf::make(rsp_params);
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_Report_h