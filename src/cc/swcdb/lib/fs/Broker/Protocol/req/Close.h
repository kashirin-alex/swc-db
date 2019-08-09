/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Close_h
#define swc_lib_fs_Broker_Protocol_req_Close_h

#include "Base.h"
#include "../params/Close.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Close : public Base {

  public:

  bool  state;
  Close(SmartFdPtr &smartfd, Callback::CloseCb_t cb=0) 
        : smartfd(smartfd), cb(cb) {

    HT_DEBUGF("close %s", smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_CLOSE, 20000);
    Params::CloseReq params(smartfd->fd());
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

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_CLOSE, &ptr, &remain))
      return;

    smartfd->fd(-1);
    smartfd->pos(0);
    HT_DEBUGF("close %s error='%d'", smartfd->to_string().c_str(), error);
    
    cb(error, smartfd);
  }

  private:
  SmartFdPtr           smartfd;
  Callback::CloseCb_t  cb;
};
typedef std::shared_ptr<Close> ClosePtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Close_h