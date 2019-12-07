/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_RgrUpdate_h
#define swc_app_manager_handlers_RgrUpdate_h

#include "swcdb/db/Protocol/Mngr/params/RgrUpdate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void rgr_update(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RgrUpdate params;
    params.decode(&ptr, &remain);
      
    // std::cout << params.to_string() << "\n";
    conn->response_ok(ev);
    Env::Rangers::get()->update_status(params.hosts, params.sync_all);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}
  

}}}}

#endif // swc_app_manager_handlers_RgrUpdate_h