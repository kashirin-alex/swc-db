/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_ColumnDelete_h
#define swc_app_rangeserver_handlers_ColumnDelete_h

#include "swcdb/lib/db/Protocol/params/ColumnId.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {

class ColumnDelete : public AppHandler {
  public:

  ColumnDelete(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::ColumnId params;
      params.decode(&ptr, &remain);

      int err = Error::OK;
      Env::RsColumns::get()->remove(err, params.cid,
        [this, cid=params.cid](int err){
          Env::Schemas::get()->remove(cid);
          if(err == Error::OK)
            m_conn->response_ok(m_ev); // cb->run();
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

#endif // swc_app_rangeserver_handlers_ColumnDelete_h