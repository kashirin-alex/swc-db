/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Append.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Append::Append(uint32_t timeout, SmartFd::Ptr& smartfd, 
               StaticBuffer& buffer, Flags flags, 
               const Callback::AppendCb_t& cb)
              : amount(0), smartfd(smartfd), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "append flags=%d timeout=%d amount=%lu %s", 
            flags, timeout, buffer.size, smartfd->to_string().c_str());

  cbp = CommBuf::make(
    Params::AppendReq(smartfd->fd(), (uint8_t)flags),
    buffer
  );
  cbp->header.set(Cmd::FUNCTION_APPEND, timeout);
}

std::promise<void> Append::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise]
       (int, const SmartFd::Ptr&, size_t){ await->set_value(); };
  return r_promise;
}

void Append::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_APPEND, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      Params::AppendRsp params;
      params.decode(&ptr, &remain);
      amount = params.amount;
      smartfd->pos(params.offset + amount);
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }

  SWC_LOGF(LOG_DEBUG, "append %s amount='%lu' error='%d'", 
            smartfd->to_string().c_str(), amount, error);
  
  cb(error, smartfd, amount);
}



}}}}
