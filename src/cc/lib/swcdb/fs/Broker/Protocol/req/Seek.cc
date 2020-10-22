/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Seek.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Seek::Seek(uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t offset,
           const FS::Callback::SeekCb_t& cb)
          : Base(Buffers::make(Params::SeekReq(smartfd->fd(), offset))),
            smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_SEEK, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}


void Seek::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_seek(ev, smartfd);
  cb(error, smartfd);
}



}}}}}
