/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/AppendSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


AppendSync::AppendSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd,
                       StaticBuffer& buffer, FS::Flags flags)
                      : amount(0), smartfd(smartfd) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("append flags=%d amount=%lu timeout=%d ", 
                    flags, buffer.size, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  cbp = Buffers::make(
    Params::AppendReq(smartfd->fd(), (uint8_t)flags),
    buffer
  );
  cbp->header.set(FUNCTION_APPEND, timeout);
}

void AppendSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_append(ev, smartfd, amount);
  BaseSync::acknowledge();
}



}}}}}
