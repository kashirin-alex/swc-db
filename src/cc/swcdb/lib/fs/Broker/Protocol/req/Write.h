/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Write_h
#define swc_lib_fs_Broker_Protocol_req_Write_h

#include "Base.h"
#include "../params/Write.h"
namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Write : public Base {

  public:
  
  Write(uint32_t timeout, SmartFdPtr &smartfd, 
        int32_t replication, int64_t blksz, StaticBuffer &buffer,
        Callback::WriteCb_t cb=0) 
        : smartfd(smartfd), cb(cb) {

    HT_DEBUGF("write %s", smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_WRITE, timeout);
    cbp = CommBuf::make(
      header, 
      Params::WriteReq(smartfd->filepath(), smartfd->flags(), 
                       replication, blksz, buffer.size), 
      buffer
    );
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFdPtr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_WRITE, &ptr, &remain))
      return;

    smartfd->fd(-1);
    smartfd->pos(0);
    HT_DEBUGF("write %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFdPtr           smartfd;
  Callback::WriteCb_t  cb;
};
typedef std::shared_ptr<Write> WritePtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Write_h