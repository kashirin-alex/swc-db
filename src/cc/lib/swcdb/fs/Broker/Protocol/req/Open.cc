/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Open.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Open::Open(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr& smartfd, 
           int32_t bufsz, const Callback::OpenCb_t& cb) 
          : fs(fs), smartfd(smartfd), cb(cb) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("open timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM); 
  );

  cbp = CommBuf::make(
    Params::OpenReq(smartfd->filepath(), smartfd->flags(), bufsz));
  cbp->header.set(Cmd::FUNCTION_OPEN, timeout);
}

std::promise<void> Open::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, const SmartFd::Ptr&){ await->set_value(); };
  return r_promise;
}

void Open::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_OPEN, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("open fds-open=%lu ", fs->fds_open());
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  cb(error, smartfd);
}



}}}}
