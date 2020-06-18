/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_ColumnUpdate_h
#define swc_manager_Protocol_handlers_ColumnUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/ColumnUpdate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void column_update(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnUpdate params;
    params.decode(&ptr, &remain);
      
    conn->response_ok(ev);
      
    Env::Mngr::mngd_columns()->update_status(
      params.function, params.schema, params.err);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}

  

}}}}

#endif // swc_manager_Protocol_handlers_ColumnUpdate_h