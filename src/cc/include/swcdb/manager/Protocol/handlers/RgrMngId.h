/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_RgrMngId_h
#define swc_manager_Protocol_handlers_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void rgr_mng_id(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RgrMngId req_params;
    req_params.decode(&ptr, &remain);

    if(!Env::Mngr::role()->is_active_role(Types::MngrRole::RANGERS)) {
      SWC_LOG_OUT(LOG_DEBUG, 
        SWC_LOG_OSTREAM << "MNGR NOT ACTIVE, flag=" << req_params.flag
                        << " rgrid=" << req_params.rgrid;
        req_params.print(SWC_LOG_OSTREAM << ' ');
      );
        
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
        rgrid_t rgrid = rangers->rgr_set_id(req_params.endpoints);

        SWC_LOG_OUT(LOG_DEBUG, 
          SWC_LOG_OSTREAM << "RS_REQ, rgrid=" << req_params.rgrid;
          req_params.print(SWC_LOG_OSTREAM << ' ');
        );

        auto cbp = CommBuf::make(
          Params::RgrMngId(rgrid, Params::RgrMngId::Flag::MNGR_ASSIGNED)
        );
        cbp->header.initialize_from_request_header(ev->header);
        conn->send_response(cbp);
        break;
      }

      case Params::RgrMngId::Flag::RS_ACK: {
        if(rangers->rgr_ack_id(req_params.rgrid, req_params.endpoints)) {
          SWC_LOG_OUT(LOG_DEBUG, 
            SWC_LOG_OSTREAM << "RS_ACK, rgrid=" << req_params.rgrid;
            req_params.print(SWC_LOG_OSTREAM << ' ');
          );
          conn->response_ok(ev);

        } else {
          SWC_LOG_OUT(LOG_DEBUG, 
            SWC_LOG_OSTREAM << "RS_ACK(MNGR_REREQ), rgrid=" << req_params.rgrid;
            req_params.print(SWC_LOG_OSTREAM << ' ');
          );
            
          auto cbp = CommBuf::make(
            Params::RgrMngId(0, Params::RgrMngId::Flag::MNGR_REREQ)
          );
          cbp->header.initialize_from_request_header(ev->header);
          conn->send_response(cbp);
        }
        break;
      }

      case Params::RgrMngId::Flag::RS_DISAGREE: {
        rgrid_t rgrid = rangers->rgr_had_id(req_params.rgrid, req_params.endpoints);
          SWC_LOG_OUT(LOG_DEBUG, 
            SWC_LOG_OSTREAM << "RS_DISAGREE, rgr_had_id=" << req_params.rgrid 
              << " > rgrid=" << rgrid;
            req_params.print(SWC_LOG_OSTREAM << ' ');
          );

        if(rgrid) {
          auto cbp = CommBuf::make(
            Params::RgrMngId(rgrid, Params::RgrMngId::Flag::MNGR_REASSIGN)
          );
          cbp->header.initialize_from_request_header(ev->header);
          conn->send_response(cbp);
        } else {
          conn->response_ok(ev);
        }
        break;
      }

      case Params::RgrMngId::Flag::RS_SHUTTINGDOWN: {
        rangers->rgr_shutdown(req_params.rgrid, req_params.endpoints);

        SWC_LOG_OUT(LOG_DEBUG, 
          SWC_LOG_OSTREAM << "RS_SHUTTINGDOWN, rgrid=" << req_params.rgrid;
          req_params.print(SWC_LOG_OSTREAM << ' ');
        );
      
        auto cbp = CommBuf::make(
          Params::RgrMngId(
            req_params.rgrid, Params::RgrMngId::Flag::RS_SHUTTINGDOWN)
        );
        cbp->header.initialize_from_request_header(ev->header);
        conn->send_response(cbp);
        break;
      }
        
      default:
        break;
    }

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}

#endif // swc_manager_Protocol_handlers_RgrMngId