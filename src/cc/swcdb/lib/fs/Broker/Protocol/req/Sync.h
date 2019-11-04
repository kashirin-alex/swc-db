/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Sync_h
#define swc_lib_fs_Broker_Protocol_req_Sync_h

#include "Base.h"
#include "../params/Sync.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Sync : public Base {

  public:
  
  Sync(uint32_t timeout, SmartFdPtr &smartfd, Callback::SyncCb_t cb=0) 
      : smartfd(smartfd), cb(cb) {

    HT_DEBUGF("sync %s", smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_SYNC, timeout);    
    cbp = CommBuf::make(header, Params::SyncReq(smartfd->fd()));
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFdPtr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_SYNC, &ptr, &remain))
      return;

    HT_DEBUGF("sync %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFdPtr          smartfd;
  Callback::SyncCb_t  cb;
};
typedef std::shared_ptr<Sync> SyncPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Sync_h