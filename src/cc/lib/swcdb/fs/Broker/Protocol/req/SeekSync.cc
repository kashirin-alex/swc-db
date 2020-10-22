/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/SeekSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


SeekSync::SeekSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t offset)
                  : Base(Buffers::make(Params::SeekReq(smartfd->fd(), offset))),
                    smartfd(smartfd) {
  cbp->header.set(FUNCTION_SEEK, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void SeekSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 
  Base::handle_seek(ev, smartfd);
  BaseSync::acknowledge();
}



}}}}}
