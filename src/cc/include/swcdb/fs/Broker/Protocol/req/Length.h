/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Length_h
#define swc_fs_Broker_Protocol_req_Length_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Length.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Length : public Base {

  public:

  size_t length;

  Length(uint32_t timeout, const std::string& name, 
         const Callback::LengthCb_t& cb=0) 
        : length(0), name(name), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());

    cbp = CommBuf::make(Params::LengthReq(name));
    cbp->header.set(Cmd::FUNCTION_LENGTH, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int, size_t){ await->set_value(); };
    return r_promise;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(ev, Cmd::FUNCTION_LENGTH, &ptr, &remain))
      return;

    if(!error) {
      Params::LengthRsp params;
      params.decode(&ptr, &remain);
      length = params.length;
    }

    SWC_LOGF(LOG_DEBUG, "length path='%s' error='%d' length='%lu'",
             name.c_str(), error, length);
    
    cb(error, length);
  }

  private:
  const std::string     name;
  Callback::LengthCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Length_h