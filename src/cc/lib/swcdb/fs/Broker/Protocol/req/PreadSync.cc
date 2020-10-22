/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/PreadSync.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


PreadSync::PreadSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
                     uint64_t offset, void* dst, size_t len, bool allocated)
                    : Base(Buffers::make(Params::PreadReq(
                        smartfd->fd(), offset, len))), 
                      buffer(dst), allocated(allocated), amount(0), 
                      smartfd(smartfd) {
  cbp->header.set(FUNCTION_PREAD, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read offset=%lu len=%lu timeout=%d ", 
                    offset, len, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void PreadSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_pread(ev, smartfd, amount);
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
