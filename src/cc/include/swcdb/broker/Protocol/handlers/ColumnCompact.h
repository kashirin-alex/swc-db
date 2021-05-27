/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnCompact_h
#define swcdb_broker_Protocol_handlers_ColumnCompact_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {



class ColumnCompact final : public Mngr::Req::ColumnCompact_Base {
  public:

  ConnHandlerPtr  conn;
  Event::Ptr      ev;

  ColumnCompact(const SWC::client::Clients::Ptr& clients,
                const Mngr::Params::ColumnCompactReq& params,
                const ConnHandlerPtr& conn, const Event::Ptr& ev)
                : Mngr::Req::ColumnCompact_Base(
                    clients, params, ev->header.timeout_ms),
                  conn(conn), ev(ev) {
  }

  virtual ~ColumnCompact() { }

  bool valid() override {
    return !ev->expired() && conn->is_open();
  }

  void callback(const Mngr::Params::ColumnCompactRsp& rsp) override {
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

    std::make_shared<ColumnCompact>(
      Env::Clients::get(), params, conn, ev)->run();

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