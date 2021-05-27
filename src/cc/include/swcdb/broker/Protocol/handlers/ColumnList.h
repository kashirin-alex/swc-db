/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnList_h
#define swcdb_broker_Protocol_handlers_ColumnList_h


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


class ColumnList final : public Mngr::Req::ColumnList_Base {
  public:

  ConnHandlerPtr  conn;
  Event::Ptr      ev;

  ColumnList(const SWC::client::Clients::Ptr& clients,
            const Mngr::Params::ColumnListReq& params,
            const ConnHandlerPtr& conn, const Event::Ptr& ev)
            : Mngr::Req::ColumnList_Base(
                clients, params, ev->header.timeout_ms),
              conn(conn), ev(ev) {
  }

  virtual ~ColumnList() { }

  bool valid() override {
    return !ev->expired() && conn->is_open();
  }

  void callback(int err, const Mngr::Params::ColumnListRsp& rsp) override {
    if(valid()) {
      auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rsp, 4);
      cbp->append_i32(
        err == Error::CLIENT_STOPPING ? Error::SERVER_SHUTTING_DOWN : err);
      conn->send_response(cbp);
    }
    if(!err)
      Env::Clients::get()->schemas.set(rsp.schemas);
    Env::Bkr::processed();
  }

};


void column_list(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Mngr::Params::ColumnListReq params;
    params.decode(&ptr, &remain);

    std::make_shared<ColumnList>(
      Env::Clients::get(), params, conn, ev)->run();

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );

    auto cbp =  Buffers::make(ev, 4);
    cbp->append_i32(e.code());
    conn->send_response(cbp);
    Env::Bkr::processed();
  }
}



}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnList_h
