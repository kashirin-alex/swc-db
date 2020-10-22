/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/FlushSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


FlushSync::FlushSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd)
                    : Base(Buffers::make(Params::FlushReq(smartfd->fd()))),
                      smartfd(smartfd) {
  cbp->header.set(FUNCTION_FLUSH, timeout);
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("flush timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}


void FlushSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_flush(ev, smartfd);
  BaseSync::acknowledge();
}



}}}}}
