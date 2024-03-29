/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnMng_h
#define swcdb_broker_Protocol_handlers_ColumnMng_h


#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {



struct ColumnMng  {

  ConnHandlerPtr  conn;
  Event::Ptr      ev;
  DB::Schema::Ptr schema;

  SWC_CAN_INLINE
  ColumnMng(const ConnHandlerPtr& a_conn, const Event::Ptr& a_ev,
            const DB::Schema::Ptr& a_schema) noexcept
            : conn(a_conn), ev(a_ev), schema(a_schema) {
  }

  ~ColumnMng() noexcept { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }

  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open() && Env::Bkr::is_accepting();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&, int err) {
    if(!ev->expired() && conn->is_open()) {
      if(err == Error::CLIENT_STOPPING || !Env::Bkr::is_accepting())
        err = Error::SERVER_SHUTTING_DOWN;
      err ? conn->send_error(err, "", ev) : conn->response_ok(ev);
    }
    schema->cid == DB::Schema::NO_CID
      ? Env::Clients::get()->schemas.remove(schema->col_name)
      : Env::Clients::get()->schemas.remove(schema->cid);
    Env::Bkr::processed();
  }

};


void column_mng(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Mngr::Params::ColumnMng params;
    params.decode(&ptr, &remain);

    Mngr::Req::ColumnMng<ColumnMng>::request(
      params, ev->header.timeout_ms, conn, ev, params.schema);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code() , "", ev);
    Env::Bkr::processed();
  }
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnMng_h
