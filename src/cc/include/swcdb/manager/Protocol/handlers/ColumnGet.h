/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnGet_h
#define swcdb_manager_Protocol_handlers_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void mngr_update_response(const ConnHandlerPtr& conn, const Event::Ptr& ev,
                          int err, Params::ColumnGetReq::Flag flag,
                          const DB::Schema::Ptr& schema) {
  if(!err && !schema)
    err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;

  auto cbp = err
    ? Buffers::make(ev, 4)
    : Buffers::make(ev, Params::ColumnGetRsp(flag, schema), 4);
  cbp->append_i32(err);
  conn->send_response(cbp);
}



class MngrColumnGet : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<MngrColumnGet> Ptr;

  SWC_CAN_INLINE
  MngrColumnGet(const ConnHandlerPtr& a_conn, const Event::Ptr& a_ev,
                Params::ColumnGetReq::Flag a_flag,
                const Params::ColumnGetReq& params)
                : client::ConnQueue::ReqBase(
                    Buffers::make(
                      params, 0, COLUMN_GET, a_ev->header.timeout_ms)
                  ),
                  conn(a_conn), ev(a_ev), flag(a_flag) {
  }

  virtual ~MngrColumnGet() noexcept { }

  bool valid() override {
    return !ev->expired() &&
            conn->is_open() &&
            Env::Mngr::mngd_columns()->running();
  }

  void handle_no_conn() override {
    if(valid())
      request_again();
    else if(!Env::Mngr::mngd_columns()->running())
      mngr_update_response(
        conn, ev, Error::SERVER_SHUTTING_DOWN, flag, nullptr);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& a_ev) override {
    Params::ColumnGetRsp params;
    int err = a_ev->response_code();
    if(!err) {
      try {
        const uint8_t *ptr = a_ev->data.base + 4;
        size_t remain = a_ev->data.size - 4;
        params.decode(&ptr, &remain);
        if(params.schema &&
           Env::Mngr::mngd_columns()->is_active(params.schema->cid)) {
          int tmperr;
          Env::Mngr::schemas()->add(tmperr, params.schema);
        }
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        err = e.code();
      }
    }

    if(valid())
      mngr_update_response(conn, ev, err, flag, params.schema);
  }

  private:
  ConnHandlerPtr                   conn;
  Event::Ptr                       ev;
  Mngr::Params::ColumnGetReq::Flag flag;

};


DB::Schema::Ptr get_schema(int &err, const Params::ColumnGetReq& params) {
  switch(params.flag) {
    case Params::ColumnGetReq::Flag::SCHEMA_BY_ID:
      return Env::Mngr::schemas()->get(params.cid);

    case Params::ColumnGetReq::Flag::SCHEMA_BY_NAME:
      return Env::Mngr::schemas()->get(params.name);

    case Params::ColumnGetReq::Flag::ID_BY_NAME:
      return Env::Mngr::schemas()->get(params.name);

    default:
      err = Error::COLUMN_UNKNOWN_GET_FLAG;
      return nullptr;
  }
}

void column_get(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnGetReq params;
    params.decode(&ptr, &remain);

    int err = Error::OK;
    DB::Schema::Ptr schema;
    if(Env::Mngr::mngd_columns()->is_schemas_mngr(err)) {
      if(!err)
        schema = get_schema(err, params);
      return mngr_update_response(conn, ev, err, params.flag, schema);
    }

    schema = get_schema(err, params);
    if(schema || err)
      return mngr_update_response(conn, ev, err, params.flag, schema);

    auto flag = params.flag;
    if(flag == Params::ColumnGetReq::Flag::ID_BY_NAME)
      params.flag = Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

    Env::Mngr::role()->req_mngr_inchain(
      MngrColumnGet::Ptr(new MngrColumnGet(conn, ev, flag, params)));

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    mngr_update_response(
      conn, ev, e.code(), Params::ColumnGetReq::Flag::ID_BY_NAME, nullptr);
  }

}



}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnGet_h
