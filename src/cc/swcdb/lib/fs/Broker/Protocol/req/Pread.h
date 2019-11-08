/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Pread_h
#define swc_lib_fs_Broker_Protocol_req_Pread_h

#include "Base.h"
#include "../params/Pread.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Pread : public Base {

  public:
  
  void*   buffer;
  bool    allocated;
  size_t  amount;
  
  Pread(uint32_t timeout, SmartFd::Ptr &smartfd, 
        uint64_t offset, void* dst, size_t len, bool allocated,
        Callback::PreadCb_t cb=0)
      : smartfd(smartfd), buffer(dst), allocated(allocated), 
        cb(cb), amount(0) {
    HT_DEBUGF("pread offset=%d len=%d timeout=%d %s", 
              offset, len, timeout, smartfd->to_string().c_str());
    cbp = CommBuf::make(Params::PreadReq(smartfd->fd(), offset, len));
    cbp->header.set(Cmd::FUNCTION_PREAD, timeout);
  }

  std::promise<void> promise() {
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, SmartFd::Ptr smartfd, StaticBuffer::Ptr buf){
           await->set_value();
          };
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_PREAD, &ptr, &remain))
      return;

    StaticBuffer::Ptr buf = nullptr;
    if(error == Error::OK || error == Error::FS_EOF) {
      Params::ReadRsp params;
      params.decode(&ptr, &remain);
      amount = ev->data_ext.size;
      smartfd->pos(params.offset+amount);

      if(amount > 0) {
        if(buffer == nullptr) {
          buf = std::make_shared<StaticBuffer>(ev->data_ext); 
        } else {
          if(allocated)
            memcpy(buffer, ev->data_ext.base, amount);
          else
            ((StaticBuffer*)buffer)->set(ev->data_ext);
        }
      }
    }

    HT_DEBUGF("pread %s amount='%d' error='%d'", 
              smartfd->to_string().c_str(), amount, error);

    cb(error, smartfd, buf);
  }

  private:
  SmartFd::Ptr        smartfd;
  Callback::PreadCb_t cb;
};



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Pread_h