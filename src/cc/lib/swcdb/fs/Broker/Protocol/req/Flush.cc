/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Flush.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Flush::Flush(uint32_t timeout, SmartFd::Ptr& smartfd, 
             const Callback::FlushCb_t& cb) 
            : smartfd(smartfd), cb(cb) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("flush timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  cbp = CommBuf::make(Params::FlushReq(smartfd->fd()));
  cbp->header.set(Cmd::FUNCTION_FLUSH, timeout);
}

std::promise<void> Flush::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, const SmartFd::Ptr&){ await->set_value(); };
  return r_promise;
}

void Flush::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_FLUSH, &ptr, &remain))
    return;

  switch(error) {
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    Error::print(SWC_LOG_OSTREAM << "flush ", error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );

  cb(error, smartfd);
}



}}}}
