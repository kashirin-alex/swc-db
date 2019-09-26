/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngrState_h
#define swc_app_manager_handlers_MngrState_h


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class MngrState : public AppHandler {
  public:

  MngrState(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Params::MngrState req_params;
      req_params.decode(&ptr, &remain);

      bool new_active_columns = Env::MngrRole::get()->fill_states(
        req_params.states, req_params.token, 
        nullptr // std::make_shared<ResponseCallback>(m_conn, m_ev)
      ); 

      Env::MngrRole::get()->update_manager_addr(
        m_conn->endpoint_remote_hash(), req_params.mngr_host);

      m_conn->response_ok(m_ev);

      if(Env::MngrRole::get()->require_sync())
        Env::Rangers::get()->require_sync();

      if(new_active_columns)
        Env::Rangers::get()->new_columns();

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_manager_handlers_MngrState_h