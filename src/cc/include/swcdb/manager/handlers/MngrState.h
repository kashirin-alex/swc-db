/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngrState_h
#define swc_app_manager_handlers_MngrState_h


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void mngr_state(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::MngrState req_params;
    req_params.decode(&ptr, &remain);

    bool new_active_columns = Env::MngrRole::get()->fill_states(
      req_params.states, req_params.token, 
      nullptr // std::make_shared<ResponseCallback>(conn, ev)
    ); 

    Env::MngrRole::get()->update_manager_addr(
      conn->endpoint_remote_hash(), req_params.mngr_host);

    conn->response_ok(ev);

    if(Env::MngrRole::get()->require_sync())
      Env::Rangers::get()->require_sync();

    if(new_active_columns)
      Env::Rangers::get()->new_columns();

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}
  

}}}}

#endif // swc_app_manager_handlers_MngrState_h