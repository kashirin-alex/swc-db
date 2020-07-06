/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Seek_h
#define swc_fs_Broker_Protocol_req_Seek_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Seek.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Seek : public Base {

  public:
  
  Seek(uint32_t timeout, SmartFd::Ptr& smartfd, size_t offset,
       const Callback::SeekCb_t& cb=0) 
      : smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "seek offset=%lu %s", 
             offset, smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::SeekReq(smartfd->fd(), offset));
    cbp->header.set(Cmd::FUNCTION_SEEK, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise] (int, const SmartFd::Ptr&){ await->set_value(); };
    return r_promise;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(ev, Cmd::FUNCTION_SEEK, &ptr, &remain))
      return;

    switch(error) {
      case Error::OK: {
        Params::SeekRsp params;
        params.decode(&ptr, &remain);
        smartfd->pos(params.offset);
        break;
      }
      case EBADR:
      case Error::FS_BAD_FILE_HANDLE:
        smartfd->fd(-1);
      default:
        break;
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

#endif  // swc_fs_Broker_Protocol_req_Seek_h