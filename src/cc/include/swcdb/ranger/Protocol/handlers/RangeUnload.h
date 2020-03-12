/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_Protocol_handlers_RangeUnload_h
#define swc_ranger_Protocol_handlers_RangeUnload_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_unload(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColRangeId params;
    params.decode(&ptr, &remain);

    int err = Error::OK;
    RangerEnv::columns()->unload_range(err, params.cid, params.rid, 
      [conn, ev](int err){
        if(err == Error::OK)
          conn->response_ok(ev); 
          // + remove cid if no ranges left
        else
          conn->send_error(err, "", ev);
      }
    );
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_RangeUnload_h