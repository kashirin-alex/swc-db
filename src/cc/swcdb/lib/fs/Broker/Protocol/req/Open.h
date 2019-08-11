/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Open_h
#define swc_lib_fs_Broker_Protocol_req_Open_h

#include "Base.h"
#include "../params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Open : public Base {

  public:

  Open(uint32_t timeout, SmartFdPtr &smartfd, int32_t bufsz, Callback::OpenCb_t cb=0) 
       : smartfd(smartfd), cb(cb) {

    HT_DEBUGF("open %s", smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_OPEN, timeout);
    Params::OpenReq params(smartfd->filepath(), smartfd->flags(), bufsz);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, SmartFdPtr smartfd){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_OPEN, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.get_fd());
      smartfd->pos(0);
    }

    HT_DEBUGF("open %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFdPtr          smartfd;
  Callback::OpenCb_t  cb;
};
typedef std::shared_ptr<Open> OpenPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Open_h