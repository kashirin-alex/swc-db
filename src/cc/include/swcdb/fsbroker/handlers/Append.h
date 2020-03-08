/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_fsbroker_handlers_Append_h
#define swc_fsbroker_handlers_Append_h

#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace FsBroker { namespace Handler {


void append(ConnHandlerPtr conn, Event::Ptr ev) {

  int err = Error::OK;
  size_t amount = 0;
  size_t offset = 0;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::AppendReq params;
    params.decode(&ptr, &remain);
      
    auto smartfd = Env::Fds::get()->select(params.fd);
      
    if(smartfd == nullptr)
      err = EBADR;
    else {
      offset = smartfd->pos();
      amount = Env::FsInterface::fs()->append(
        err, smartfd, ev->data_ext, (FS::Flags)params.flags);
    }
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(
      FS::Protocol::Params::AppendRsp(offset, amount), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

}


}}}

#endif // swc_fsbroker_handlers_Append_h