/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Remove_h
#define swc_lib_fs_Broker_Protocol_req_Remove_h

#include "Base.h"
#include "../params/Remove.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Remove : public Base {

  public:

  Remove(uint32_t timeout, const String &name, Callback::RemoveCb_t cb=0) 
        : name(name), cb(cb) {
    HT_DEBUGF("remove path='%s'", name.c_str());

    cbp = CommBuf::make(Params::RemoveReq(name));
    cbp->header.set(Cmd::FUNCTION_REMOVE, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_REMOVE, &ptr, &remain))
      return;

    HT_DEBUGF("remove path='%s' error='%d'", name.c_str(), error);
    
    cb(error);
  }

  private:
  const String          name;
  Callback::RemoveCb_t  cb;
};



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Remove_h