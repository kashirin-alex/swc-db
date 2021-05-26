/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnGet_h
#define swcdb_broker_Protocol_handlers_ColumnGet_h

#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


DB::Schema::Ptr get_schema(
        int &err,
        const Protocol::Mngr::Params::ColumnGetReq& params) {
  switch(params.flag) {
    case Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_ID:
      return Env::Clients::get()->get_schema(err, params.cid);

    case Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME:
      return Env::Clients::get()->get_schema(err, params.name);

    case Protocol::Mngr::Params::ColumnGetReq::Flag::ID_BY_NAME:
      return Env::Clients::get()->get_schema(err, params.name);

    default:
      err = Error::COLUMN_UNKNOWN_GET_FLAG;
      return nullptr;
  }
}

void column_get(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Protocol::Mngr::Params::ColumnGetReq params;
  Protocol::Mngr::Params::ColumnGetRsp rsp_params;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    params.decode(&ptr, &remain);
    rsp_params.flag = params.flag;
    rsp_params.schema = get_schema(err, params);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = err
    ? Buffers::make(ev, 4)
    : Buffers::make(ev, rsp_params, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

  Env::Bkr::processed();
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnGet_h
