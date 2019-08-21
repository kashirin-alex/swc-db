/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngColumn_h
#define swc_app_manager_handlers_MngColumn_h

#include "swcdb/lib/db/Protocol/params/MngColumn.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class MngColumn : public AppHandler {
  public:

    MngColumn(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev){}

  void run() override {

    int err = Error::OK;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::MngColumn req_params;
      const uint8_t *base = ptr;
      req_params.decode(&ptr, &remain);

      
      if(!EnvMngrRoleState::get()->is_active(1)){
        std::cout << "MNGR NOT ACTIVE: \n";
        err = Error::MNGR_NOT_ACTIVE;
      }

      if(err == Error::OK) {
        switch(req_params.function){

          case Protocol::Params::MngColumn::Function::ADD: {
            EnvRangeServers::get()->add_column(req_params.schema, err);
            break;
          }

          default:
            break;
        }
      }
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }

    try{
      if(err == Error::OK)
        m_conn->response_ok(m_ev);
      else
        m_conn->send_error(err , "", m_ev);

      /* 
      Protocol::Params::MngColumnRsp rsp_params(err);
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, rsp_params.encoded_length());
      rsp_params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
      */
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_manager_handlers_MngColumn_h