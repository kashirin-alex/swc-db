/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_ColumnUpdate_h
#define swc_app_manager_handlers_ColumnUpdate_h

#include "swcdb/lib/db/Protocol/Mngr/params/ColumnUpdate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class ColumnUpdate : public AppHandler {
  public:

    ColumnUpdate(ConnHandlerPtr conn, Event::Ptr ev)
                : AppHandler(conn, ev){}

  void run() override {
    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      Params::ColumnUpdate params;
      params.decode(&ptr, &remain);
      
      m_conn->response_ok(m_ev);
      
      Env::Rangers::get()->update_status(
        params.function, params.schema, params.err);

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
  }

};
  

}}}}

#endif // swc_app_manager_handlers_ColumnUpdate_h