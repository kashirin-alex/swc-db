/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnCompact_h
#define swcdb_broker_Protocol_handlers_ColumnCompact_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


struct ColumnCompact {

  ConnHandlerPtr conn;
  Event::Ptr     ev;

  SWC_CAN_INLINE
  ColumnCompact(const ConnHandlerPtr& a_conn, const Event::Ptr& a_ev) noexcept
                : conn(a_conn), ev(a_ev) {
  }

  ~ColumnCompact() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }

  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open() && Env::Bkr::is_accepting();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&,
                const Mngr::Params::ColumnCompactRsp& rsp) {
    if(!ev->expired() && conn->is_open()) {
      int err = rsp.err;
      if(err == Error::CLIENT_STOPPING || !Env::Bkr::is_accepting())
        err = Error::SERVER_SHUTTING_DOWN;
      conn->send_response(
        Buffers::make(ev, Mngr::Params::ColumnCompactRsp(err)));
    }
    Env::Bkr::processed();
  }

};


void column_compact(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Mngr::Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    Mngr::Req::ColumnCompact<ColumnCompact>::request(
      params, ev->header.timeout_ms, conn, ev);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_response(
      Buffers::make(ev, Mngr::Params::ColumnCompactRsp(e.code())));
    Env::Bkr::processed();
  }

}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnCompact_h