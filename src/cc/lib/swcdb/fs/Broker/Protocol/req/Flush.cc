/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Flush.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Flush::Flush(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
             const FS::Callback::FlushCb_t& cb) 
            : Base(Buffers::make(Params::FlushReq(smartfd->fd()))),
              smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_FLUSH, timeout);
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("flush timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void Flush::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_FLUSH, &ptr, &remain))
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



}}}}}
