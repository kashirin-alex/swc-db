/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Pread_h
#define swc_app_fsbroker_handlers_Pread_h

#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace server { namespace FsBroker { namespace Handler {


void pread(ConnHandlerPtr conn, Event::Ptr ev) {

    int err = Error::OK;
    size_t offset = 0;
    StaticBuffer rbuf;
    try {

      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      FS::Protocol::Params::PreadReq params;
      params.decode(&ptr, &remain);

      auto smartfd = Env::Fds::get()->select(params.fd);
      
      if(smartfd == nullptr)
        err = EBADR;
      else {
        offset = params.offset;
        rbuf.reallocate(params.amount);
        rbuf.size = Env::FsInterface::fs()->pread(
          err, smartfd, params.offset, rbuf.base, params.amount);
      }
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  
    try {
      auto cbp = CommBuf::make(FS::Protocol::Params::ReadRsp(offset), rbuf, 4);
      cbp->header.initialize_from_request_header(ev->header);
      cbp->append_i32(err);
      conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }

}
  

}}}}

#endif // swc_app_fsbroker_handlers_Pread_h