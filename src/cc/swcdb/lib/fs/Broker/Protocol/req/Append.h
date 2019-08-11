/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Append_h
#define swc_lib_fs_Broker_Protocol_req_Append_h

#include "Base.h"
#include "../params/Append.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Append : public Base {

  public:
  
  size_t amount;
  
  Append(uint32_t timeout, SmartFdPtr &smartfd, 
        StaticBuffer &buffer, Flags flags, Callback::AppendCb_t cb=0) 
        : smartfd(smartfd), cb(cb), amount(0) {

    HT_DEBUGF("append flags=%d timeout=%d %s", 
              flags, timeout, smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_APPEND, timeout);
    Params::AppendReq params(smartfd->fd(), buffer.size, (uint8_t)flags);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length(), buffer));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, SmartFdPtr smartfd, size_t n){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_APPEND, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::AppendRsp params;
      params.decode(&ptr, &remain);
      size_t offset = params.get_offset();
      amount = params.get_amount();
      smartfd->pos(offset+amount);
    }

    HT_DEBUGF("append %s amount='%d' error='%d'", 
              smartfd->to_string().c_str(), amount, error);
    
    cb(error, smartfd, amount);
  }

  private:
  SmartFdPtr            smartfd;
  Callback::AppendCb_t  cb;
};
typedef std::shared_ptr<Append> AppendPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Append_h