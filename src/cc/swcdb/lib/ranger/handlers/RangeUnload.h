/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeUnload_h
#define swc_app_ranger_handlers_RangeUnload_h

#include "swcdb/lib/db/Protocol/Common/params/ColRangeId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


class RangeUnload : public AppHandler {
  public:

  RangeUnload(ConnHandlerPtr conn, Event::Ptr ev)
             : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      Common::Params::ColRangeId params;
      params.decode(&ptr, &remain);

      int err = Error::OK;
      Env::RgrColumns::get()->unload_range(err, params.cid, params.rid, 
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

#endif // swc_app_ranger_handlers_RangeUnload_h