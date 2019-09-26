/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_RgrMngId_h
#define swc_app_manager_handlers_RgrMngId_h

#include "swcdb/lib/db/Protocol/Mngr/params/RgrMngId.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class RgrMngId : public AppHandler {
  public:

    RgrMngId(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev){}

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Params::RgrMngId req_params;
      req_params.decode(&ptr, &remain);

      // ResponseCallbackPtr cb = 
      //  std::make_shared<ResponseCallback>(m_conn, m_ev);
      
      
      if(!Env::MngrRole::get()->is_active(1)){
        HT_DEBUGF("MNGR NOT ACTIVE, flag=%d id=%d %s",
                  req_params.flag, req_params.id, 
                  req_params.to_string().c_str());
      
        Params::RgrMngId rsp_params(
          0, Params::RgrMngId::Flag::MNGR_NOT_ACTIVE);
        
        CommHeader header;
        header.initialize_from_request_header(m_ev->header);
        CommBufPtr cbp = std::make_shared<CommBuf>(
          header, rsp_params.encoded_length());
        rsp_params.encode(cbp->get_data_ptr_address());

        m_conn->send_response(cbp);
        return;
      }

      server::Mngr::RangersPtr rangers = Env::Rangers::get();
      switch(req_params.flag){

        case Params::RgrMngId::Flag::RS_REQ: {
          uint64_t id = rangers->rs_set_id(req_params.endpoints);

          HT_DEBUGF("RS_REQ, id=%d %s",
                    req_params.id, req_params.to_string().c_str());

          Params::RgrMngId rsp_params(
            id, Params::RgrMngId::Flag::MNGR_ASSIGNED);
          CommHeader header;
          header.initialize_from_request_header(m_ev->header);
          CommBufPtr cbp = std::make_shared<CommBuf>(
            header, rsp_params.encoded_length());
          rsp_params.encode(cbp->get_data_ptr_address());

          m_conn->send_response(cbp);
          break;
        }

        case Params::RgrMngId::Flag::RS_ACK: {
          if(rangers->rs_ack_id(req_params.id, req_params.endpoints)){
            HT_DEBUGF("RS_ACK, id=%d %s",
                      req_params.id, req_params.to_string().c_str());
            m_conn->response_ok(m_ev);

          } else {
            HT_DEBUGF("RS_ACK(MNGR_REREQ) id=%d %s",
                      req_params.id, req_params.to_string().c_str());

            Params::RgrMngId rsp_params(
              0, Params::RgrMngId::Flag::MNGR_REREQ);
            CommHeader header;
            header.initialize_from_request_header(m_ev->header);
            CommBufPtr cbp = std::make_shared<CommBuf>(
              header, rsp_params.encoded_length());
            rsp_params.encode(cbp->get_data_ptr_address());

            m_conn->send_response(cbp);
          }
          break;
        }

        case Params::RgrMngId::Flag::RS_DISAGREE: {
          uint64_t id = rangers->rs_had_id(req_params.id, 
                                                   req_params.endpoints);
          HT_DEBUGF("RS_DISAGREE, rs_had_id=%d > id=%d %s", 
                    req_params.id, id, req_params.to_string().c_str());

          if (id != 0){
            Params::RgrMngId rsp_params(
              id, Params::RgrMngId::Flag::MNGR_REASSIGN);
      
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

        case Params::RgrMngId::Flag::RS_SHUTTINGDOWN: {
          rangers->rs_shutdown(req_params.id, req_params.endpoints);

          HT_DEBUGF("RS_SHUTTINGDOWN, id=%d %s",
                    req_params.id, req_params.to_string().c_str());
          
          Params::RgrMngId rsp_params(
            req_params.id, Params::RgrMngId::Flag::RS_SHUTTINGDOWN);
      
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

#endif // swc_app_manager_handlers_RgrMngId