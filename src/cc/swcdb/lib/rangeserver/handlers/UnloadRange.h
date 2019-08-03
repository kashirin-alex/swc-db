/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_UnloadRange_h
#define swc_app_rangeserver_handlers_UnloadRange_h

#include "swcdb/lib/db/Protocol/params/ColRangeId.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {

class UnloadRange : public AppHandler {
  public:

  UnloadRange(ConnHandlerPtr conn, EventPtr ev)
             : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::ColRangeId params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      EnvRsColumns::get()->unload_range(params.cid, params.rid, 
        [this](bool state){
          m_conn->response_ok(m_ev);
        }
      );
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_UnloadRange_h