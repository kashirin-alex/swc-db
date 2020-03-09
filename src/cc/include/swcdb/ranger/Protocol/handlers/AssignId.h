/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_ranger_Protocol_handlers_AssignId_h
#define swc_ranger_Protocol_handlers_AssignId_h


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void assign_id(ConnHandlerPtr conn, Event::Ptr ev, 
               Protocol::Mngr::Req::RgrMngId::Ptr id_mngr) {
  try {

    if(RangerEnv::is_shuttingdown())
      conn->send_error(Error::SERVER_SHUTTING_DOWN, "", ev);
    else 
      conn->response_ok(ev);

    id_mngr->request();

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}


}}}}

#endif // swc_ranger_Protocol_handlers_AssignId_h