/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_manager_Protocol_handlers_RgrMngId_h
#define swc_manager_Protocol_handlers_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void rgr_mng_id(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RgrMngId req_params;
    req_params.decode(&ptr, &remain);

    // ResponseCallback::Ptr cb = 
    //  std::make_shared<ResponseCallback>(conn, ev);
          
    if(!Env::Mngr::role()->is_active(1)){
      SWC_LOGF(LOG_DEBUG, "MNGR NOT ACTIVE, flag=%d id=%d %s",
                req_params.flag, req_params.id, 
                req_params.to_string().c_str());
        
      auto cbp = CommBuf::make(
        Params::RgrMngId(0, Params::RgrMngId::Flag::MNGR_NOT_ACTIVE)
      );
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
      return;
    }

    auto rangers = Env::Mngr::rangers();
    switch(req_params.flag) {

      case Params::RgrMngId::Flag::RS_REQ: {
        uint64_t id = rangers->rgr_set_id(req_params.endpoints);

        SWC_LOGF(LOG_DEBUG, "RS_REQ, id=%d %s",
                  req_params.id, req_params.to_string().c_str());

        auto cbp = CommBuf::make(
          Params::RgrMngId(id, Params::RgrMngId::Flag::MNGR_ASSIGNED)
        );
        cbp->header.initialize_from_request_header(ev->header);
        conn->send_response(cbp);
        break;
      }

      case Params::RgrMngId::Flag::RS_ACK: {
        if(rangers->rgr_ack_id(req_params.id, req_params.endpoints)){
          SWC_LOGF(LOG_DEBUG, "RS_ACK, id=%d %s",
                    req_params.id, req_params.to_string().c_str());
          conn->response_ok(ev);

        } else {
          SWC_LOGF(LOG_DEBUG, "RS_ACK(MNGR_REREQ) id=%d %s",
                    req_params.id, req_params.to_string().c_str());
            
          auto cbp = CommBuf::make(
            Params::RgrMngId(0, Params::RgrMngId::Flag::MNGR_REREQ)
          );
          cbp->header.initialize_from_request_header(ev->header);
          conn->send_response(cbp);
        }
        break;
      }

      case Params::RgrMngId::Flag::RS_DISAGREE: {
        uint64_t id = rangers->rgr_had_id(req_params.id, req_params.endpoints);
        SWC_LOGF(LOG_DEBUG, "RS_DISAGREE, rgr_had_id=%d > id=%d %s", 
                  req_params.id, id, req_params.to_string().c_str());

        if(id) {
          auto cbp = CommBuf::make(
            Params::RgrMngId(id, Params::RgrMngId::Flag::MNGR_REASSIGN)
          );
          cbp->header.initialize_from_request_header(ev->header);
          conn->send_response(cbp);
        } else {
          conn->response_ok(ev);
        }
        break;
      }

      case Params::RgrMngId::Flag::RS_SHUTTINGDOWN: {
        rangers->rgr_shutdown(req_params.id, req_params.endpoints);

        SWC_LOGF(LOG_DEBUG, "RS_SHUTTINGDOWN, id=%d %s",
                  req_params.id, req_params.to_string().c_str());
      
        auto cbp = CommBuf::make(
          Params::RgrMngId(
            req_params.id, Params::RgrMngId::Flag::RS_SHUTTINGDOWN)
        );
        cbp->header.initialize_from_request_header(ev->header);
        conn->send_response(cbp);
        break;
      }
        
      default:
        break;
    }

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }  
}
  

}}}}

#endif // swc_manager_Protocol_handlers_RgrMngId