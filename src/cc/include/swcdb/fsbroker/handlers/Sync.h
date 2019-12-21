/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Sync_h
#define swc_app_fsbroker_handlers_Sync_h

#include "swcdb/fs/Broker/Protocol/params/Sync.h"


namespace SWC { namespace server { namespace FsBroker { namespace Handler {


void sync(ConnHandlerPtr conn, Event::Ptr ev) {

  int err = Error::OK;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::SyncReq params;
    params.decode(&ptr, &remain);

    auto smartfd = Env::Fds::get()->select(params.fd);
      
    if(smartfd == nullptr)
      err = EBADR;
    else
      Env::FsInterface::fs()->sync(err, smartfd);
  }
  catch (Exception &e) {
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
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

}
  

}}}}

#endif // swc_app_fsbroker_handlers_Sync_h