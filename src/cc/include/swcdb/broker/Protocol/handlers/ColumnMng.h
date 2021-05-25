/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnMng_h
#define swcdb_broker_Protocol_handlers_ColumnMng_h

#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {



class ColumnMng final : public Protocol::Mngr::Req::ColumnMng_Base {
  public:

  ConnHandlerPtr  conn;
  Event::Ptr      ev;
  DB::Schema::Ptr schema;

  ColumnMng(const SWC::client::Clients::Ptr& clients,
            const Protocol::Mngr::Params::ColumnMng& params,
            const ConnHandlerPtr& conn, const Event::Ptr& ev)
            : Protocol::Mngr::Req::ColumnMng_Base(
                clients, params, ev->header.timeout_ms),
              conn(conn), ev(ev), schema(params.schema) {
  }

  virtual ~ColumnMng() { }

  void callback(int err) override {
    err ? conn->send_error(err , "", ev) : conn->response_ok(ev);

    schema->cid == DB::Schema::NO_CID
      ? Env::Clients::get()->schemas.remove(schema->col_name)
      : Env::Clients::get()->schemas.remove(schema->cid);
    Env::Bkr::in_process(-1);
  }

};


void column_mng(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Protocol::Mngr::Params::ColumnMng params;
    params.decode(&ptr, &remain);

    std::make_shared<ColumnMng>(
      Env::Clients::get(), params, conn, ev)->run();

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code() , "", ev);
    Env::Bkr::in_process(-1);
  }
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnMng_h
