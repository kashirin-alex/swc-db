/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_IsRangeLoaded_h
#define swc_app_rangeserver_handlers_IsRangeLoaded_h

#include "swcdb/lib/db/Protocol/params/IsRangeLoaded.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class IsRangeLoaded : public AppHandler {
  public:

  IsRangeLoaded(ConnHandlerPtr conn, EventPtr ev)
               : AppHandler(conn, ev){ }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::IsRangeLoaded params;
      params.decode(&ptr, &remain);

      int err = Error::OK;
      RangePtr range =  Env::RsColumns::get()->get_range(
        err, params.cid, params.rid, false);
      
      if(range != nullptr && range->is_loaded()){
        m_conn->response_ok(m_ev);
      } else {
        if(err == Error::OK)
          err = Error::RS_NOT_LOADED_RANGE;
        m_conn->send_error(err, "", m_ev);
      }
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_IsRangeLoaded_h