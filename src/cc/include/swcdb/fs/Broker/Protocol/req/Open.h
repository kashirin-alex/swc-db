/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Open_h
#define swc_fs_Broker_Protocol_req_Open_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Open : public Base {

  public:

  Open(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr& smartfd, 
       int32_t bufsz, const Callback::OpenCb_t& cb=0) 
      : fs(fs), smartfd(smartfd), cb(cb) {
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

    if(!error) {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();
    }

    SWC_LOGF(LOG_DEBUG, "open %s error='%d' fds-open=%lld", 
             smartfd->to_string().c_str(), error, fs->fds_open());
    
    cb(error, smartfd);
  }

  private:
  FileSystem::Ptr     fs;
  SmartFd::Ptr        smartfd;
  Callback::OpenCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Open_h