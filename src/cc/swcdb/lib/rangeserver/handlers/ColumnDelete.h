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
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      Env::RsColumns::get()->remove(params.cid, 
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

#endif // swc_app_rangeserver_handlers_ColumnDelete_h