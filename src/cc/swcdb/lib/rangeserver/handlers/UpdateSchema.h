/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_UpdateSchema_h
#define swc_app_rangeserver_handlers_UpdateSchema_h

#include "swcdb/lib/db/Protocol/params/RsUpdateSchema.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class UpdateSchema : public AppHandler {
  public:

  UpdateSchema(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::RsUpdateSchema params;
      params.decode(&ptr, &remain);

      Env::Schemas::get()->replace(params.schema);
      HT_DEBUGF("updated %s", params.schema->to_string().c_str());
      
      m_conn->response_ok(m_ev);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_UpdateSchema_h