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
  
  size_t amount;
  void* buffer;
  
  Pread(uint32_t timeout, SmartFdPtr &smartfd, 
        uint64_t offset, void* dst, size_t len, 
        Callback::PreadCb_t cb=0)
      : smartfd(smartfd), buffer(dst), cb(cb), amount(0) {

    HT_DEBUGF("pread offset=%d len=%d timeout=%d %s", 
              offset, len, timeout, smartfd->to_string().c_str());

    CommHeader header(Cmd::FUNCTION_PREAD, timeout);
    Params::PreadReq params(smartfd->fd(), offset, len);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, SmartFdPtr smartfd, StaticBufferPtr buf){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_PREAD, &ptr, &remain))
      return;

    StaticBufferPtr buf = nullptr;
    if(error == Error::OK) {
      Params::ReadRsp params;
      params.decode(&ptr, &remain);
      size_t offset = params.get_offset();
      amount = params.get_amount();
      smartfd->pos(offset+amount);

      if(remain != amount) {
        error = Error::SERIALIZATION_INPUT_OVERRUN;
      } else {
        if(buffer == nullptr) {
          buf = std::make_shared<StaticBuffer>(remain); 
          buffer = buf->base;
        }
        memcpy(buffer, ptr, remain);
      }
    }

    HT_DEBUGF("pread %s amount='%d' error='%d'", 
              smartfd->to_string().c_str(), amount, error);

    cb(error, smartfd, buf);
  }

  private:
  SmartFdPtr          smartfd;
  Callback::PreadCb_t cb;
};
typedef std::shared_ptr<Pread> PreadPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Pread_h