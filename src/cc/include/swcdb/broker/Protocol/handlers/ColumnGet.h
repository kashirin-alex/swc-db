/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnGet_h
#define swcdb_broker_Protocol_handlers_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


struct ColumnGet {

  ConnHandlerPtr                   conn;
  Event::Ptr                       ev;
  Mngr::Params::ColumnGetReq::Flag flag;

  SWC_CAN_INLINE
  ColumnGet(const ConnHandlerPtr& a_conn,
            const Event::Ptr& a_ev,
            Mngr::Params::ColumnGetReq::Flag a_flag)
            : conn(a_conn), ev(a_ev), flag(a_flag) {
  }

  ~ColumnGet() noexcept { }

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
                int err, const Mngr::Params::ColumnGetRsp& rsp) {
    if(!ev->expired() && conn->is_open()) {
      if(err == Error::CLIENT_STOPPING || !Env::Bkr::is_accepting())
        err = Error::SERVER_SHUTTING_DOWN;
      auto cbp = err
        ? Buffers::make(ev, 4)
        : Buffers::make(ev, Mngr::Params::ColumnGetRsp(flag, rsp.schema), 4);
      cbp->append_i32(err);
      conn->send_response(cbp);
    }
    if(!err)
      Env::Clients::get()->schemas.set(rsp.schema);
    Env::Bkr::processed();
  }

};


SWC_CAN_INLINE
DB::Schema::Ptr get_schema(int &err,
                           const Mngr::Params::ColumnGetReq& params) {
  switch(params.flag) {
    case Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_ID:
      return Env::Clients::get()->schemas.get(params.cid);
    case Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME:
    case Mngr::Params::ColumnGetReq::Flag::ID_BY_NAME:
      return Env::Clients::get()->schemas.get(params.name);
    default:
      err = Error::COLUMN_UNKNOWN_GET_FLAG;
      return nullptr;
  }
}


void column_get(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Mngr::Params::ColumnGetReq params;
    params.decode(&ptr, &remain);

    auto schema = get_schema(err, params);
    if(err)
      goto _send_error;

    if(schema) {
      auto cbp = Buffers::make(
        ev, Mngr::Params::ColumnGetRsp(params.flag, schema), 4);
      cbp->append_i32(Error::OK);
      conn->send_response(cbp);
      Env::Bkr::processed();

    } else {
      auto flag = params.flag;
      if(flag == Mngr::Params::ColumnGetReq::Flag::ID_BY_NAME)
        params.flag = Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

      Mngr::Req::ColumnGet<ColumnGet>::request(
        params, ev->header.timeout_ms, conn, ev, flag);
    }
    return;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  _send_error:
    auto cbp =  Buffers::make(ev, 4);
    cbp->append_i32(err);
    conn->send_response(cbp);
    Env::Bkr::processed();
}



}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnGet_h
