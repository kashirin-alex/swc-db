/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RgrUpdate_h
#define swcdb_manager_Protocol_handlers_RgrUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/RgrUpdate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void rgr_update(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RgrUpdate params;
    params.decode(&ptr, &remain);
      
    conn->response_ok(ev);
    Env::Mngr::rangers()->update_status(params.hosts, params.sync_all);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}

#endif // swcdb_manager_Protocol_handlers_RgrUpdate_h