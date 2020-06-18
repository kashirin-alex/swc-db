/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Flush_h
#define swc_fs_Broker_Protocol_req_Flush_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Flush.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Flush : public Base {

  public:

  Flush(uint32_t timeout, SmartFd::Ptr& smartfd, 
        const Callback::FlushCb_t& cb=0) 
        : smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "flush %s", smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::FlushReq(smartfd->fd()));
    cbp->header.set(Cmd::FUNCTION_FLUSH, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_FLUSH, &ptr, &remain))
      return;

    switch(error) {
      case EBADR:
      case Error::FS_BAD_FILE_HANDLE:
        smartfd->fd(-1);
      default:
        break;
    }
    
    SWC_LOGF(LOG_DEBUG, "flush %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFd::Ptr         smartfd;
  Callback::FlushCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Flush_h