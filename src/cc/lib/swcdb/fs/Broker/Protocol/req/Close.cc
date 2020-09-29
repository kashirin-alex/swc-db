/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Close.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Close::Close(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr& smartfd, 
            const Callback::CloseCb_t& cb)
            : fs(fs), smartfd(smartfd), cb(cb) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("close timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM); 
  );
 
  cbp = Comm::CommBuf::make(Params::CloseReq(smartfd->fd()));
  cbp->header.set(Cmd::FUNCTION_CLOSE, timeout);
}

std::promise<void> Close::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, const SmartFd::Ptr&){ await->set_value(); };
  return r_promise;
}

void Close::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_CLOSE, &ptr, &remain))
    return;

  smartfd->fd(-1);
  smartfd->pos(0);
  fs->fd_open_decr();

  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("close fds-open=%lu ", fs->fds_open());
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  cb(error, smartfd);
}



}}}}
