/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Sync_h
#define swc_fs_Broker_Protocol_req_Sync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Sync.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Sync : public Base {

  public:
  
  Sync(uint32_t timeout, SmartFd::Ptr& smartfd, 
       const Callback::SyncCb_t& cb=0) 
      : smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "sync %s", smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::SyncReq(smartfd->fd()));
    cbp->header.set(Cmd::FUNCTION_SYNC, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_SYNC, &ptr, &remain))
      return;

    switch(error) {
      case Error::OK:
        break;
      case EBADR:
      case Error::FS_BAD_FILE_HANDLE:
        smartfd->fd(-1);
      default:
        break;
    }
    
    SWC_LOGF(LOG_DEBUG, "sync %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFd::Ptr        smartfd;
  Callback::SyncCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Sync_h