/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_AssignRsId_h
#define swc_app_manager_handlers_AssignRsId_h

#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/manager/RangeServers.h"

#include "swcdb/lib/db/Protocol/Commands.h"
#include "swcdb/lib/db/Protocol/params/AssignRsID.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class AssignRsId : public AppHandler {
  public:

    AssignRsId(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev){}

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::AssignRsID req_params;
      const uint8_t *base = ptr;
      req_params.decode(&ptr, &remain);

      // ResponseCallbackPtr cb = 
      //  std::make_shared<ResponseCallback>(m_conn, m_ev);
      
      // std::cout << "AssignRsId-run rs " << req_params.to_string() << "\n";
      
      if(!EnvMngrRoleState::get()->is_active(1)){
        std::cout << "MNGR NOT ACTIVE: \n";
      
        Protocol::Params::AssignRsID rsp_params(
          0, Protocol::Params::AssignRsID::Flag::MNGR_NOT_ACTIVE, {});
        
        CommHeader header;
        header.initialize_from_request_header(m_ev->header);
        CommBufPtr cbp = std::make_shared<CommBuf>(
          header, rsp_params.encoded_length());
        rsp_params.encode(cbp->get_data_ptr_address());

        m_conn->send_response(cbp);
        return;
      }

      RangeServersPtr  rangeservers = EnvRangeServers::get();
      std::cout << "MNGR ACTIVE: flag="<<req_params.flag<<"\n";
      switch(req_params.flag){

        case Protocol::Params::AssignRsID::Flag::RS_REQ: {
          uint64_t rs_id = rangeservers->rs_set_id(req_params.endpoints);

          Protocol::Params::AssignRsID rsp_params(
            rs_id, Protocol::Params::AssignRsID::Flag::MNGR_ASSIGNED, {});
          CommHeader header;
          header.initialize_from_request_header(m_ev->header);
          CommBufPtr cbp = std::make_shared<CommBuf>(
            header, rsp_params.encoded_length());
          rsp_params.encode(cbp->get_data_ptr_address());

          m_conn->send_response(cbp);
          break;
        }

        case Protocol::Params::AssignRsID::Flag::RS_ACK: {
          if(rangeservers->rs_ack_id(req_params.rs_id, req_params.endpoints)){
            m_conn->response_ok(m_ev);

          } else {

            Protocol::Params::AssignRsID rsp_params(
              0, Protocol::Params::AssignRsID::Flag::MNGR_REREQ, {});
            CommHeader header;
            header.initialize_from_request_header(m_ev->header);
            CommBufPtr cbp = std::make_shared<CommBuf>(
              header, rsp_params.encoded_length());
            rsp_params.encode(cbp->get_data_ptr_address());

            m_conn->send_response(cbp);
          }
          break;
        }

        case Protocol::Params::AssignRsID::Flag::RS_DISAGREE: {
          uint64_t rs_id = rangeservers->rs_had_id(
            req_params.rs_id, req_params.endpoints);

          if (rs_id != 0){
            Protocol::Params::AssignRsID rsp_params(
              rs_id, Protocol::Params::AssignRsID::Flag::MNGR_REASSIGN, {});
      
            CommHeader header;
            header.initialize_from_request_header(m_ev->header);
            CommBufPtr cbp = std::make_shared<CommBuf>(
              header, rsp_params.encoded_length());
            rsp_params.encode(cbp->get_data_ptr_address());

            m_conn->send_response(cbp);
          } else {
            m_conn->response_ok(m_ev);
          }

          break;
        }

        case Protocol::Params::AssignRsID::Flag::RS_SHUTTINGDOWN: {
          rangeservers->rs_shutdown(req_params.rs_id, req_params.endpoints);
          m_conn->response_ok(m_ev);
          break;
        }
        
        default:
          break;
      }

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
    
    std::cout << EnvRangeServers::get()->to_string() << "\n";
    
  }

};
  

}}}}

#endif // swc_app_manager_handlers_AssignRsId_h