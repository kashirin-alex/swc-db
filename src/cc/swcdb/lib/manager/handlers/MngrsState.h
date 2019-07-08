/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngrsState_h
#define swc_app_manager_handlers_MngrsStatee_h



namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class MngrsState : public AppHandler {
  public:

  MngrsState(ConnHandlerPtr conn, EventPtr ev, RoleStatePtr role_state)
              : AppHandler(conn, ev), m_role_state(role_state) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::MngrsState req_params;
      const uint8_t *base = ptr;
      req_params.decode(&ptr, &remain);

      ResponseCallbackPtr cb = std::make_shared<ResponseCallback>(m_conn, m_ev);
      
      std::cout << " token=" << req_params.token << " cb=" << (size_t)cb.get() << " fill_states\n";

      m_role_state->fill_states(
        req_params.states, req_params.token,
        cb 
      );
      m_conn->response_ok(m_ev);

    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

  private:
  RoleStatePtr      m_role_state;
};
  

}}}}

#endif // swc_app_manager_handlers_IsMngrActive_h