/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_MngrActive_h
#define swcdb_manager_Protocol_handlers_MngrActive_h

#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void mngr_active(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Manager::MngrStatus::Ptr h = nullptr;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::MngrActiveReq params;
    params.decode(&ptr, &remain);

    h = params.role & DB::Types::MngrRole::COLUMNS 
      ? Env::Mngr::role()->active_mngr(params.cid)
      : Env::Mngr::role()->active_mngr_role(params.role);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

  conn->send_response(
    Buffers::make(Params::MngrActiveRsp(h ? h->endpoints : EndPoints())),
    ev
  );
}

  

}}}}}

#endif // swcdb_manager_Protocol_handlers_MngrActive_h
