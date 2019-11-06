/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngrActive_h
#define swc_app_manager_handlers_MngrActive_h

#include "swcdb/lib/db/Protocol/Mngr/params/MngrActive.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class MngrActive : public AppHandler {
  public:

  MngrActive(ConnHandlerPtr conn, Event::Ptr ev)
            : AppHandler(conn, ev){ }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      Params::MngrActiveReq params;
      params.decode(&ptr, &remain);

      server::Mngr::MngrStatusPtr h = Env::MngrRole::get()->active_mngr(
        params.begin, params.end);

      EndPoints endpoints;
      if(h!=nullptr) 
        endpoints = h->endpoints;

      auto cbp = CommBuf::make(Params::MngrActiveRsp(endpoints));
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_manager_handlers_MngrActive_h