/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_ColumnMng_h
#define swc_app_manager_handlers_ColumnMng_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


class ColumnMng : public AppHandler {
  public:

  ColumnMng(ConnHandlerPtr conn, Event::Ptr ev)
            : AppHandler(conn, ev){}

  void run() override {

    int err = Error::OK;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      Params::ColumnMng req_params;
      req_params.decode(&ptr, &remain);

      Env::Rangers::get()->is_active(err, 1, true);
      
      if(err == Error::OK) {
        Env::Rangers::get()->column_action({
          .params=req_params, 
          .cb=[conn=m_conn, ev=m_ev](int err){
            if(err == Error::OK)
              conn->response_ok(ev);
            else
              conn->send_error(err , "", ev);
            }
        });
      }

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }

    try{
      if(err != Error::OK)
        m_conn->send_error(err , "", m_ev);

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
  }

};
  

}}}}

#endif // swc_app_manager_handlers_ColumnMng_h