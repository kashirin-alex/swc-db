/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnGet_h
#define swcdb_manager_Protocol_handlers_ColumnGet_h

#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/manager/Protocol/Mngr/req/MngrColumnGet.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


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

void mngr_update_response(const ConnHandlerPtr& conn, const Event::Ptr& ev,
                          int err, Params::ColumnGetReq::Flag flag, 
                          const DB::Schema::Ptr& schema) {
  if(!err && !schema)
    err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;

  try {
    auto cbp = err 
      ? Buffers::make(4)
      : Buffers::make(Params::ColumnGetRsp(flag, schema), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}

void column_get(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  Params::ColumnGetReq::Flag flag;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnGetReq req_params;
    req_params.decode(&ptr, &remain);
    flag = req_params.flag;
      
    DB::Schema::Ptr schema = get_schema(err, req_params);
    if(schema || err)
      return mngr_update_response(conn, ev, err, flag, schema);

    if(Env::Mngr::mngd_columns()->is_schemas_mngr(err))
      return mngr_update_response(conn, ev, err, flag, schema);

    if(flag == Params::ColumnGetReq::Flag::ID_BY_NAME)
      req_params.flag = Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

    Env::Mngr::role()->req_mngr_inchain(
      std::make_shared<Req::MngrColumnGet>(
        req_params,
        [conn, ev] (int err, const Params::ColumnGetRsp& params) {
          if(!err && params.schema) {
            int tmperr;
            Env::Mngr::schemas()->add(tmperr, params.schema);
          }
          mngr_update_response(conn, ev, err, params.flag, params.schema);
        }
      )
    );

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    flag = Params::ColumnGetReq::Flag::ID_BY_NAME;
    try {
      mngr_update_response(conn, ev, e.code(), flag, nullptr);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }

}



}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnGet_h
