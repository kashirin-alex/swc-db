/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_fsbroker_handlers_Create_h
#define swc_fsbroker_handlers_Create_h

#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace FsBroker { namespace Handler {


void create(ConnHandlerPtr conn, Event::Ptr ev) {

  int err = Error::OK;
  int32_t fd = -1;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::CreateReq params;
    params.decode(&ptr, &remain);

    auto smartfd = FS::SmartFd::make_ptr(params.fname, params.flags);
 
    Env::FsInterface::fs()->create(
      err, smartfd, params.bufsz, params.replication, params.blksz
    );

    if(smartfd->valid() && err==Error::OK)
      fd = Env::Fds::get()->add(smartfd);
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(FS::Protocol::Params::OpenRsp(fd), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

}
  

}}}

#endif // swc_fsbroker_handlers_Create_h