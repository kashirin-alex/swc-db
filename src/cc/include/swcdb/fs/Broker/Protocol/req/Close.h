/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Close_h
#define swc_fs_Broker_Protocol_req_Close_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Close.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Close : public Base {

  public:

  Close(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr &smartfd, Callback::CloseCb_t cb=0)
        : fs(fs), smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "close %s", smartfd->to_string().c_str());
 
    cbp = CommBuf::make(Params::CloseReq(smartfd->fd()));
    cbp->header.set(Cmd::FUNCTION_CLOSE, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_CLOSE, &ptr, &remain))
      return;

    smartfd->fd(-1);
    smartfd->pos(0);
    fs->fd_open_decr();

    SWC_LOGF(LOG_DEBUG, "close %s error='%d' fds-open=%lld", 
             smartfd->to_string().c_str(), error, fs->fds_open());

    cb(error, smartfd);
  }

  private:
  FileSystem::Ptr      fs;
  SmartFd::Ptr         smartfd;
  Callback::CloseCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Close_h