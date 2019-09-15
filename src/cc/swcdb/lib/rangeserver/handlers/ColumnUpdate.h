/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_ColumnUpdate_h
#define swc_app_rangeserver_handlers_ColumnUpdate_h

#include "swcdb/lib/db/Protocol/params/RsColumnUpdate.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class ColumnUpdate : public AppHandler {
  public:

  ColumnUpdate(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::RsColumnUpdate params;
      params.decode(&ptr, &remain);

      Env::Schemas::get()->replace(params.schema);
      if(!Env::RsData::is_shuttingdown())
        HT_DEBUGF("updated %s", params.schema->to_string().c_str());
      
      m_conn->response_ok(m_ev);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_ColumnUpdate_h