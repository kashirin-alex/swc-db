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
  ColumnCompact(const ConnHandlerPtr& conn, const Event::Ptr& ev) noexcept
                : conn(conn), ev(ev) {
  }

  //~ColumnCompact() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }

  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&,
                const Mngr::Params::ColumnCompactRsp& rsp) {
    if(valid())
      conn->send_response(Buffers::make(
        ev,
        rsp.err == Error::CLIENT_STOPPING
          ? Mngr::Params::ColumnCompactRsp(Error::SERVER_SHUTTING_DOWN)
          : rsp
      ));
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