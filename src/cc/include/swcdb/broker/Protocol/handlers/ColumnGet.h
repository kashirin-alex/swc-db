/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnGet_h
#define swcdb_broker_Protocol_handlers_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


class ColumnGet final : public Mngr::Req::ColumnGet_Base {
  public:

  ConnHandlerPtr                   conn;
  Event::Ptr                       ev;
  Mngr::Params::ColumnGetReq::Flag flag;

  ColumnGet(const SWC::client::Clients::Ptr& clients,
            Mngr::Params::ColumnGetReq::Flag flag,
            const Mngr::Params::ColumnGetReq& params,
            const ConnHandlerPtr& conn, const Event::Ptr& ev)
            : Mngr::Req::ColumnGet_Base(
                clients, params, ev->header.timeout_ms),
              conn(conn), ev(ev), flag(flag) {
  }

  virtual ~ColumnGet() { }

  bool valid() override {
    return !ev->expired() && conn->is_open();
  }

  void callback(int err, const Mngr::Params::ColumnGetRsp& rsp) override {
    if(valid()) {
      auto cbp = err
        ? Buffers::make(ev, 4)
        : Buffers::make(ev, Mngr::Params::ColumnGetRsp(flag, rsp.schema), 4);
      cbp->append_i32(
        err == Error::CLIENT_STOPPING ? Error::SERVER_SHUTTING_DOWN : err);
      conn->send_response(cbp);
    }
    if(!err)
      Env::Clients::get()->schemas.set(rsp.schema);
    Env::Bkr::processed();
  }

};


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

    } else {
      auto flag = params.flag;
      if(flag == Mngr::Params::ColumnGetReq::Flag::ID_BY_NAME)
        params.flag = Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

      std::make_shared<ColumnGet>(
        Env::Clients::get(), flag, params, conn, ev)->run();
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
