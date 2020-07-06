/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Remove_h
#define swc_fs_Broker_Protocol_req_Remove_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Remove.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Remove : public Base {

  public:

  Remove(uint32_t timeout, const std::string& name, 
         const Callback::RemoveCb_t& cb=0) 
        : name(name), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "remove path='%s'", name.c_str());

    cbp = CommBuf::make(Params::RemoveReq(name));
    cbp->header.set(Cmd::FUNCTION_REMOVE, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int){ await->set_value(); };
    return r_promise;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(ev, Cmd::FUNCTION_REMOVE, &ptr, &remain))
      return;

    SWC_LOGF(LOG_DEBUG, "remove path='%s' error='%d'", name.c_str(), error);
    
    cb(error);
  }

  private:
  const std::string     name;
  Callback::RemoveCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Remove_h