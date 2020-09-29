/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_MngrActive_h
#define swc_manager_Protocol_handlers_MngrActive_h

#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void mngr_active(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {
  Manager::MngrStatus::Ptr h = nullptr;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::MngrActiveReq params;
    params.decode(&ptr, &remain);

    h = params.role & Types::MngrRole::COLUMNS 
      ? Env::Mngr::role()->active_mngr(params.cid)
      : Env::Mngr::role()->active_mngr_role(params.role);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

  try {
    auto cbp = Comm::Buffers::make(
      Params::MngrActiveRsp(h ? h->endpoints : Comm::EndPoints()) );
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}

  

}}}}

#endif // swc_manager_Protocol_handlers_MngrActive_h