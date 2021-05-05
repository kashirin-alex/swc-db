/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RgrMngId_h
#define swcdb_manager_Protocol_handlers_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void rgr_mng_id(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Params::RgrMngId rsp_params;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RgrMngId req_params;
    req_params.decode(&ptr, &remain);

    if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::RANGERS)) {
      SWC_LOG_OUT(LOG_DEBUG,
        SWC_LOG_OSTREAM << "MNGR NOT ACTIVE, flag=" << req_params.flag
                        << " rgrid=" << req_params.rgrid;
        req_params.print(SWC_LOG_OSTREAM << ' ');
      );
      rsp_params.flag = Params::RgrMngId::Flag::MNGR_NOT_ACTIVE;
      goto send_response;
    }

    auto rangers = Env::Mngr::rangers();
    switch(req_params.flag) {

      case Params::RgrMngId::Flag::RS_REQ: {
        rsp_params.rgrid = rangers->rgr_set_id(req_params.endpoints);
        rsp_params.flag = Params::RgrMngId::Flag::MNGR_ASSIGNED;
        rsp_params.fs = Env::FsInterface::interface()->get_type_underlying();

        SWC_LOG_OUT(LOG_DEBUG,
          SWC_LOG_OSTREAM << "RS_REQ, rgrid=" << rsp_params.rgrid;
          req_params.print(SWC_LOG_OSTREAM << ' ');
        );
        break;
      }

      case Params::RgrMngId::Flag::RS_ACK: {
        if(rangers->rgr_ack_id(req_params.rgrid, req_params.endpoints)) {
          SWC_LOG_OUT(LOG_DEBUG,
            SWC_LOG_OSTREAM
              << "RS_ACK, rgrid=" << req_params.rgrid;
            req_params.print(SWC_LOG_OSTREAM << ' ');
          );
          rsp_params.flag = Params::RgrMngId::Flag::MNGR_ACK;

        } else {
          SWC_LOG_OUT(LOG_DEBUG,
            SWC_LOG_OSTREAM
              << "RS_ACK(MNGR_REREQ), rgrid=" << req_params.rgrid;
            req_params.print(SWC_LOG_OSTREAM << ' ');
          );
          rsp_params.flag = Params::RgrMngId::Flag::MNGR_REREQ;
        }
        break;
      }

      case Params::RgrMngId::Flag::RS_DISAGREE: {
        rsp_params.rgrid = rangers->rgr_had_id(
          req_params.rgrid, req_params.endpoints);

        rsp_params.flag = rsp_params.rgrid
          ? Params::RgrMngId::Flag::MNGR_REASSIGN
          : Params::RgrMngId::Flag::MNGR_ACK;

        SWC_LOG_OUT(LOG_DEBUG,
          SWC_LOG_OSTREAM << "RS_DISAGREE, rgr_had_id=" << req_params.rgrid
            << " > rgrid=" << rsp_params.rgrid;
          req_params.print(SWC_LOG_OSTREAM << ' ');
        );
        break;
      }

      case Params::RgrMngId::Flag::RS_SHUTTINGDOWN: {
        rangers->rgr_shutdown(req_params.rgrid, req_params.endpoints);

        SWC_LOG_OUT(LOG_DEBUG,
          SWC_LOG_OSTREAM << "RS_SHUTTINGDOWN, rgrid=" << req_params.rgrid;
          req_params.print(SWC_LOG_OSTREAM << ' ');
        );

        rsp_params.flag = Params::RgrMngId::Flag::RS_SHUTTINGDOWN;
        rsp_params.rgrid = req_params.rgrid;
        break;
      }

      default: {
        err = Error::NOT_IMPLEMENTED;
        SWC_LOG_OUT(LOG_WARN,
          SWC_LOG_OSTREAM << "NOT_IMPLEMENTED flag=" << req_params.flag;
        );
        break;
      }
    }

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  send_response:
    auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rsp_params, 4);
    cbp->append_i32(err);
    conn->send_response(cbp);

}


}}}}}

#endif // swcdb_manager_Protocol_handlers_RgrMngId
