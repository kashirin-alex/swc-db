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
      params.decode(&ptr, &remain);

      int err = Error::OK;
      Env::RsColumns::get()->unload_range(err, params.cid, params.rid, 
        [this](int err){
          if(err == Error::OK)
            m_conn->response_ok(m_ev); 
            // + remove cid if no ranges left
          else
            m_conn->send_error(err, "", m_ev);
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