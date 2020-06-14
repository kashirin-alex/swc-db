/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_ColumnGet_h
#define swc_manager_Protocol_handlers_ColumnGet_h

#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/manager/Protocol/Mngr/req/MngrColumnGet.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


DB::Schema::Ptr get_schema(int &err, Params::ColumnGetReq params) {
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

void mngr_update_response(ConnHandlerPtr conn, Event::Ptr ev,
                          int err, Params::ColumnGetReq::Flag flag, 
                          DB::Schema::Ptr schema) {
  if(err == Error::OK && schema == nullptr)
    err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;

  try {
    auto cbp = err ? 
        CommBuf::make(4)
      : CommBuf::make(Params::ColumnGetRsp(flag, schema), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}

void column_get(ConnHandlerPtr conn, Event::Ptr ev) {

  int err = Error::OK;
  Params::ColumnGetReq::Flag flag;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnGetReq req_params;
    req_params.decode(&ptr, &remain);
    flag = req_params.flag;
      
    DB::Schema::Ptr schema = get_schema(err, req_params);
    if(schema != nullptr || err) {
      mngr_update_response(conn, ev, err, flag, schema);
      return;
    }

    if(!Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS))
      err = Error::MNGR_NOT_ACTIVE;

    if(!err) {
      mngr_update_response(conn, ev, err, flag, schema);
      return;
    }

    if(flag == Params::ColumnGetReq::Flag::ID_BY_NAME)
      req_params.flag = Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

    Env::Mngr::role()->req_mngr_inchain(
      std::make_shared<Req::MngrColumnGet>(
        req_params,
        [conn, ev](int err, const Params::ColumnGetRsp& params){
          if(err == Error::OK && params.schema != nullptr){
            int tmperr;
            Env::Mngr::schemas()->add(tmperr, params.schema);
          }
          mngr_update_response(conn, ev, err, params.flag, params.schema);
        }
      )
    );
    return;

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }
    
  mngr_update_response(conn, ev, err, flag, nullptr);
}



}}}}

#endif // swc_manager_Protocol_handlers_ColumnGet_h