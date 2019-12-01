/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Seek_h
#define swc_lib_fs_Broker_Protocol_req_Seek_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Seek.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Seek : public Base {

  public:
  
  Seek(uint32_t timeout, SmartFd::Ptr &smartfd, size_t offset,
      Callback::SeekCb_t cb=0) 
      : smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "seek offset=%d %s", offset, smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::SeekReq(smartfd->fd(), offset));
    cbp->header.set(Cmd::FUNCTION_SEEK, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_SEEK, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::SeekRsp params;
      params.decode(&ptr, &remain);
      smartfd->pos(params.offset);
    }

    SWC_LOGF(LOG_DEBUG, "seek %s error='%d'", 
              smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFd::Ptr        smartfd;
  Callback::SeekCb_t  cb;
};



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Seek_h