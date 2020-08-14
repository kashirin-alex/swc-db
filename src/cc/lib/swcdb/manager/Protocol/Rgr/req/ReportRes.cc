
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/manager/Protocol/Rgr/req/ReportRes.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


ReportRes::ReportRes(const Manager::Ranger::Ptr& rgr)
                     : client::ConnQueue::ReqBase(false), rgr(rgr) {
  cbp = CommBuf::make(1);
  cbp->append_i8((uint8_t)Params::Report::Function::RESOURCES);
  cbp->header.set(REPORT, 60000);
}

ReportRes::~ReportRes() { }

void ReportRes::handle_no_conn() {  
  if(was_called)
    return;
  was_called = true;
  Env::Mngr::rangers()->rgr_report(
    rgr->rgrid, 
    Error::COMM_NOT_CONNECTED,
    Protocol::Rgr::Params::Report::RspRes()
  );
}

void ReportRes::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();
  if(was_called || ev->header.command != REPORT)
    return;
  was_called = true;
  
  Params::Report::RspRes rsp_params;
  int err = ev->type == Event::Type::ERROR ? ev->error : Error::OK;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  }
  Env::Mngr::rangers()->rgr_report(rgr->rgrid, err, rsp_params);
}


}}}}
