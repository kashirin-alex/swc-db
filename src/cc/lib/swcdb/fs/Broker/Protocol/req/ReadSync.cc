/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ReadSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


ReadSync::ReadSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd,
                   void* dst, size_t len, bool allocated)
                  : Base(Buffers::make(Params::ReadReq(smartfd->fd(), len))),
                    buffer(dst), allocated(allocated), amount(0),
                    smartfd(smartfd) {
  cbp->header.set(FUNCTION_READ, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read len=%lu timeout=%d ", len, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void ReadSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_read(ev, smartfd, amount);
  if(amount) {
    if(allocated) {
        memcpy(buffer, ev->data_ext.base, amount);
    } else {
      ((StaticBuffer*)buffer)->set(ev->data_ext);
    }
  }
  BaseSync::acknowledge();
}



}}}}}
