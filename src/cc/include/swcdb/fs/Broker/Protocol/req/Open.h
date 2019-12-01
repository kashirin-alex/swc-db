/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Open_h
#define swc_lib_fs_Broker_Protocol_req_Open_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Open : public Base {

  public:

  Open(uint32_t timeout, SmartFd::Ptr &smartfd, int32_t bufsz, 
       Callback::OpenCb_t cb=0) 
      : smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "open %s", smartfd->to_string().c_str());

    cbp = CommBuf::make(
      Params::OpenReq(smartfd->filepath(), smartfd->flags(), bufsz));
    cbp->header.set(Cmd::FUNCTION_OPEN, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_OPEN, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
    }

    SWC_LOGF(LOG_DEBUG, "open %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFd::Ptr        smartfd;
  Callback::OpenCb_t  cb;
};



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Open_h