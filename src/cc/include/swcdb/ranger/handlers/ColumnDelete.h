/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_ColumnDelete_h
#define swc_app_ranger_handlers_ColumnDelete_h

#include "swcdb/db/Protocol/Common/params/ColumnId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void column_delete(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColumnId params;
    params.decode(&ptr, &remain);

    int err = Error::OK;
    Env::RgrColumns::get()->remove(err, params.cid,
      [conn, ev](int err) {
        if(!err)
          conn->response_ok(ev); // cb->run();
        else
          conn->send_error(err, "", ev);
      }
    );
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_app_ranger_handlers_ColumnDelete_h