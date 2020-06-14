/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_ColumnMng_h
#define swc_manager_Protocol_handlers_ColumnMng_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void column_mng(ConnHandlerPtr conn, Event::Ptr ev) {
  int err = Error::OK;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnMng req_params;
    req_params.decode(&ptr, &remain);

    if(!Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS))
      err = Error::MNGR_NOT_ACTIVE;
      
    if(err == Error::OK) {
      Env::Mngr::mngd_columns()->action({
        .params=req_params, 
        .cb=[conn, ev](int err){
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
      conn->send_error(err , "", ev);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}
  

}}}}

#endif // swc_manager_Protocol_handlers_ColumnMng_h