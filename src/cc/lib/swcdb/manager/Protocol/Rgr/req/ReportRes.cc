
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/manager/Protocol/Rgr/req/ReportRes.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

ReportRes::ReportRes(const Manager::Ranger::Ptr& rgr)
                          : client::ConnQueue::ReqBase(false), 
                            rgr(rgr) {
  cbp = CommBuf::make(Params::ReportReq(Params::ReportReq::RESOURCES));
  cbp->header.set(REPORT, 60000);
}
  
ReportRes::~ReportRes() { }

void ReportRes::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(was_called || ev->header.command != REPORT)
    return;
  was_called = true;

  Protocol::Rgr::Params::ReportResRsp params;
  
  if(ev->type == Event::Type::DISCONNECT)
    params.err = Error::COMM_NOT_CONNECTED;
  else if(ev->error)
    params.err = ev->error;
  
  if(!params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      params.decode(&ptr, &remain);
    } catch(Exception& e) {
      params.err = e.code();
    }
  }
  
  Env::Mngr::rangers()->rgr_report(rgr->rgrid, params);
}
  
void ReportRes::handle_no_conn() {  
  if(was_called)
    return;
  Env::Mngr::rangers()->rgr_report(
    rgr->rgrid, 
    Protocol::Rgr::Params::ReportResRsp(Error::COMM_NOT_CONNECTED)
  );
}


}}}}
