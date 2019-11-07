/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Flush_h
#define swc_lib_fs_Broker_Protocol_req_Flush_h

#include "Base.h"
#include "../params/Flush.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Flush : public Base {

  public:

  Flush(uint32_t timeout, SmartFd::Ptr &smartfd, Callback::FlushCb_t cb=0) 
        : smartfd(smartfd), cb(cb) {
    HT_DEBUGF("flush %s", smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::FlushReq(smartfd->fd()));
    cbp->header.set(Cmd::FUNCTION_FLUSH, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_FLUSH, &ptr, &remain))
      return;

    HT_DEBUGF("flush %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFd::Ptr         smartfd;
  Callback::FlushCb_t  cb;
};



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Flush_h