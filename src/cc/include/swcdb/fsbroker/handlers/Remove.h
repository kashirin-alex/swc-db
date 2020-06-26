/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_fsbroker_handlers_Remove_h
#define swc_fsbroker_handlers_Remove_h

#include "swcdb/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace FsBroker { namespace Handler {


void remove(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
    
  int err = Error::OK;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::RemoveReq params;
    params.decode(&ptr, &remain);

    Env::FsInterface::fs()->remove(err, params.fname);

  } catch (Exception &e) {
    err = e.code();
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

}
  

}}}

#endif // swc_fsbroker_handlers_Remove_h