/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_ColumnMng_h
#define swc_app_manager_handlers_ColumnMng_h

#include "swcdb/lib/db/Protocol/params/ColumnMng.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class ColumnMng : public AppHandler {
  public:

  ColumnMng(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev){}

  void run() override {

    int err = Error::OK;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::ColumnMng req_params;
      req_params.decode(&ptr, &remain);

      Env::RangeServers::get()->is_active(err, 1);
      
      if(err == Error::OK) {
        Env::RangeServers::get()->column_action({
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
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }

    try{
      if(err != Error::OK)
        m_conn->send_error(err , "", m_ev);

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_manager_handlers_ColumnMng_h