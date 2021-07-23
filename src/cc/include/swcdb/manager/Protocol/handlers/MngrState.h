/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_MngrState_h
#define swcdb_manager_Protocol_handlers_MngrState_h


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


struct MngrState {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  MngrState(const Comm::ConnHandlerPtr& conn,
            const Comm::Event::Ptr& ev) noexcept
            : conn(conn), ev(ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      Params::MngrState req_params;
      req_params.decode(&ptr, &remain);

      Env::Mngr::role()->fill_states(
        req_params.states, req_params.token,
        nullptr // std::make_shared<ResponseCallback>(conn, ev)
      );

      Env::Mngr::role()->update_manager_addr(
        conn->endpoint_remote_hash(), req_params.mngr_host);

      conn->response_ok(ev);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      conn->send_error(e.code(), "", ev);
    }
  }

};


}}}}}

#endif // swcdb_manager_Protocol_handlers_MngrState_h
