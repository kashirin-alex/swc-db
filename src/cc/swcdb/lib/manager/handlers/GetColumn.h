/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_GetColumn_h
#define swc_app_manager_handlers_GetColumn_h

#include "swcdb/lib/db/Protocol/params/GetColumn.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class GetColumn : public AppHandler {
  public:

  GetColumn(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev){}

  void run() override {

    int err = Error::OK;
    Protocol::Params::GetColumnReq::Flag flag;
    DB::SchemaPtr schema;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::GetColumnReq req_params;
      req_params.decode(&ptr, &remain);

      
      if(!Env::MngrRole::get()->is_active(1)){
        std::cout << "MNGR NOT ACTIVE: \n";
        err = Error::MNGR_NOT_ACTIVE;
      }

      if(err == Error::OK) {
        flag = req_params.flag;
        switch(flag) {
          case Protocol::Params::GetColumnReq::Flag::SCHEMA_BY_ID: {
            schema = Env::Schemas::get()->get(req_params.cid);
            break;
          }
          case Protocol::Params::GetColumnReq::Flag::SCHEMA_BY_NAME: {
            schema = Env::Schemas::get()->get(req_params.name);
            break;
          }
          case Protocol::Params::GetColumnReq::Flag::ID_BY_NAME: {
            schema = Env::Schemas::get()->get(req_params.name);
            break;
          }
          default:
            err = Error::COLUMN_UNKNOWN_GET_FLAG;
        }
      }

      if(schema == nullptr)
        err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }


    try {
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp;

      if(err == Error::OK) {
        Protocol::Params::GetColumnRsp rsp_params(flag, schema);
        cbp = std::make_shared<CommBuf>(header, 4+rsp_params.encoded_length());
        cbp->append_i32(err);
        rsp_params.encode(cbp->get_data_ptr_address());

      } else {
        cbp = std::make_shared<CommBuf>(header, 4);
        cbp->append_i32(err);
      }

      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_manager_handlers_GetColumn_h