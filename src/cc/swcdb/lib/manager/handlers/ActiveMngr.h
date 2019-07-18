/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_ActiveMngr_h
#define swc_app_manager_handlers_ActiveMngr_h

#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/params/ActiveMngrReq.h"
#include "swcdb/lib/db/Protocol/params/ActiveMngrRsp.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class ActiveMngr : public AppHandler {
  public:

  ActiveMngr(ConnHandlerPtr conn, EventPtr ev, RoleStatePtr role_state)
              : AppHandler(conn, ev), m_role_state(role_state) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::ActiveMngrReq req_params;
      const uint8_t *base = ptr;
      req_params.decode(&ptr, &remain);

      Protocol::Params::ActiveMngrRsp rsp_params(
        (Protocol::Params::HostEndPoints*)m_role_state->active_mngr(
              req_params.begin, req_params.end).get()
      );

      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, rsp_params.encoded_length());
      rsp_params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
      
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

  private:
  RoleStatePtr      m_role_state;
};
  

}}}}

#endif // swc_app_manager_handlers_ActiveMngr_h