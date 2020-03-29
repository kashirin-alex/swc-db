/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Create_h
#define swc_fs_Broker_Protocol_req_Create_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Create : public Base {

  public:
  
  Create(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr &smartfd, 
        int32_t bufsz, uint8_t replication, int64_t blksz, 
        Callback::CreateCb_t cb=0) 
        : fs(fs), smartfd(smartfd), cb(cb) {
    SWC_LOGF(LOG_DEBUG, 
      "create %s bufsz(%d) replication(%d) blksz(%d)", 
      smartfd->to_string().c_str(), bufsz, replication, blksz);
    
    cbp = CommBuf::make(
      Params::CreateReq(smartfd->filepath(), smartfd->flags(), 
                        bufsz, replication, blksz)
    );
    cbp->header.set(Cmd::FUNCTION_CREATE, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFd::Ptr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_CREATE, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();
    }

    SWC_LOGF(LOG_DEBUG, "create %s error='%d' fds-open=%lld", 
             smartfd->to_string().c_str(), error, fs->fds_open());
    
    cb(error, smartfd);
  }

  private:
  FileSystem::Ptr       fs;
  SmartFd::Ptr          smartfd;
  Callback::CreateCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Create_h