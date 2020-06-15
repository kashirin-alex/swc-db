/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_MngrActive_h
#define swc_manager_Protocol_handlers_MngrActive_h

#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void mngr_active(ConnHandlerPtr conn, Event::Ptr ev) {
  Manager::MngrStatus::Ptr h = nullptr;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::MngrActiveReq params;
    params.decode(&ptr, &remain);

    h = params.role & Types::MngrRole::COLUMNS 
      ? Env::Mngr::role()->active_mngr(params.cid)
      : Env::Mngr::role()->active_mngr_role(params.role);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

  try {
    auto cbp = CommBuf::make(
      Params::MngrActiveRsp(h ? h->endpoints : EndPoints()) );
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}

  

}}}}

#endif // swc_manager_Protocol_handlers_MngrActive_h