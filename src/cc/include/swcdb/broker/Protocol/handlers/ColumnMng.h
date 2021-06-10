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
  ColumnMng(const ConnHandlerPtr& conn, const Event::Ptr& ev,
            const DB::Schema::Ptr& schema) noexcept
            : conn(conn), ev(ev), schema(schema) {
  }

  //~ColumnMng() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }
  
  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&, int err) {
    if(valid())
      err
        ? conn->send_error(
            err == Error::CLIENT_STOPPING ? Error::SERVER_SHUTTING_DOWN : err,
            "",
            ev
          )
        : conn->response_ok(ev);

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
