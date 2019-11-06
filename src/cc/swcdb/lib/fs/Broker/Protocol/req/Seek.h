/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Seek_h
#define swc_lib_fs_Broker_Protocol_req_Seek_h

#include "Base.h"
#include "../params/Seek.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Seek : public Base {

  public:
  
  Seek(uint32_t timeout, SmartFdPtr &smartfd, size_t offset,
      Callback::SeekCb_t cb=0) 
      : smartfd(smartfd), cb(cb) {
    HT_DEBUGF("seek offset=%d %s", offset, smartfd->to_string().c_str());

    cbp = CommBuf::make(Params::SeekReq(smartfd->fd(), offset));
    cbp->header.set(Cmd::FUNCTION_SEEK, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, SmartFdPtr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_SEEK, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::SeekRsp params;
      params.decode(&ptr, &remain);
      smartfd->pos(params.get_offset());
    }

    HT_DEBUGF("seek %s error='%d'", 
              smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFdPtr          smartfd;
  Callback::SeekCb_t  cb;
};
typedef std::shared_ptr<Seek> SeekPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Seek_h