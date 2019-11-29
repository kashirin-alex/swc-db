/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_ColumnGet_h
#define swc_app_manager_handlers_ColumnGet_h

#include "swcdb/lib/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/lib/db/Protocol/Mngr/req/MngrColumnGet.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class ColumnGet : public AppHandler {
  public:

  ColumnGet(ConnHandlerPtr conn, Event::Ptr ev)
            : AppHandler(conn, ev){}

  DB::Schema::Ptr get_schema(int &err, Params::ColumnGetReq params) {
    switch(params.flag) {
      case Params::ColumnGetReq::Flag::SCHEMA_BY_ID:
        return Env::Schemas::get()->get(params.cid);

      case Params::ColumnGetReq::Flag::SCHEMA_BY_NAME:
        return Env::Schemas::get()->get(params.name);

      case Params::ColumnGetReq::Flag::ID_BY_NAME:
        return Env::Schemas::get()->get(params.name);

      default:
        err = Error::COLUMN_UNKNOWN_GET_FLAG;
        return nullptr;
    }
  }

  void run() override {

    int err = Error::OK;
    Params::ColumnGetReq::Flag flag;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      Params::ColumnGetReq req_params;
      req_params.decode(&ptr, &remain);
      flag = req_params.flag;
      
      DB::Schema::Ptr schema = get_schema(err, req_params);
      
      if(schema != nullptr || err){
        response(m_conn, m_ev, err, flag, schema);
        return;
      }
      Env::Rangers::get()->is_active(err, 1, true);
      if(err) {
        response(m_conn, m_ev, err, flag, schema);
        return;
      }

      if(flag == Params::ColumnGetReq::Flag::ID_BY_NAME)
        req_params.flag = Params::ColumnGetReq::Flag::SCHEMA_BY_NAME;

      Env::MngrRole::get()->req_mngr_inchain(
        std::make_shared<Req::MngrColumnGet>(
          req_params,
          [conn=m_conn, ev=m_ev](int err, Params::ColumnGetRsp params){
            if(err == Error::OK && params.schema != nullptr){
              int tmperr;
              Env::Schemas::get()->add(tmperr, params.schema);
            }
            ColumnGet::response(conn, ev, err, params.flag, params.schema);
          }
        )
      );
      return;

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
    
    response(m_conn, m_ev, err, flag, nullptr);
  }

  static void response(ConnHandlerPtr conn, Event::Ptr ev,
                        int err, Params::ColumnGetReq::Flag flag, 
                        DB::Schema::Ptr schema){
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

};
  
}}}}

#endif // swc_app_manager_handlers_ColumnGet_h