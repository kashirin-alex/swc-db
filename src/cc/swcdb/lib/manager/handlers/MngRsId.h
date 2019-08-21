/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_MngRsId_h
#define swc_app_manager_handlers_MngRsId_h

#include "swcdb/lib/db/Protocol/params/MngRsId.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class MngRsId : public AppHandler {
  public:

    MngRsId(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev){}

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::MngRsId req_params;
      const uint8_t *base = ptr;
      req_params.decode(&ptr, &remain);

      // ResponseCallbackPtr cb = 
      //  std::make_shared<ResponseCallback>(m_conn, m_ev);
      
      // std::cout << "MngRsId-run rs " << req_params.to_string() << "\n";
      
      if(!EnvMngrRoleState::get()->is_active(1)){
        std::cout << "MNGR NOT ACTIVE: \n";
      
        Protocol::Params::MngRsId rsp_params(
          0, Protocol::Params::MngRsId::Flag::MNGR_NOT_ACTIVE, {});
        
        CommHeader header;
        header.initialize_from_request_header(m_ev->header);
        CommBufPtr cbp = std::make_shared<CommBuf>(
          header, rsp_params.encoded_length());
        rsp_params.encode(cbp->get_data_ptr_address());

        m_conn->send_response(cbp);
        return;
      }

      RangeServersPtr  rangeservers = EnvRangeServers::get();
      switch(req_params.flag){

        case Protocol::Params::MngRsId::Flag::RS_REQ: {
          uint64_t rs_id = rangeservers->rs_set_id(req_params.endpoints);

          Protocol::Params::MngRsId rsp_params(
            rs_id, Protocol::Params::MngRsId::Flag::MNGR_ASSIGNED, {});
          CommHeader header;
          header.initialize_from_request_header(m_ev->header);
          CommBufPtr cbp = std::make_shared<CommBuf>(
            header, rsp_params.encoded_length());
          rsp_params.encode(cbp->get_data_ptr_address());

          m_conn->send_response(cbp);
          break;
        }

        case Protocol::Params::MngRsId::Flag::RS_ACK: {
          if(rangeservers->rs_ack_id(req_params.rs_id, req_params.endpoints)){
            m_conn->response_ok(m_ev);

          } else {

            Protocol::Params::MngRsId rsp_params(
              0, Protocol::Params::MngRsId::Flag::MNGR_REREQ, {});
            CommHeader header;
            header.initialize_from_request_header(m_ev->header);
            CommBufPtr cbp = std::make_shared<CommBuf>(
              header, rsp_params.encoded_length());
            rsp_params.encode(cbp->get_data_ptr_address());

            m_conn->send_response(cbp);
          }
          break;
        }

        case Protocol::Params::MngRsId::Flag::RS_DISAGREE: {
          uint64_t rs_id = rangeservers->rs_had_id(req_params.rs_id, 
                                                   req_params.endpoints);
          HT_DEBUGF("RS_DISAGREE, rs_had_id=%d > rs_id=%d", req_params.rs_id, rs_id);

          if (rs_id != 0){
            Protocol::Params::MngRsId rsp_params(
              rs_id, Protocol::Params::MngRsId::Flag::MNGR_REASSIGN, {});
      
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

        case Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN: {
          rangeservers->rs_shutdown(req_params.rs_id, req_params.endpoints);
          
          Protocol::Params::MngRsId rsp_params(
            req_params.rs_id, Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN, {});
      
          CommHeader header;
          header.initialize_from_request_header(m_ev->header);
          CommBufPtr cbp = std::make_shared<CommBuf>(
            header, rsp_params.encoded_length());
          rsp_params.encode(cbp->get_data_ptr_address());

          m_conn->send_response(cbp);
          break;
        }
        
        default:
          break;
      }

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
    
  }

};
  

}}}}

#endif // swc_app_manager_handlers_MngRsId_h