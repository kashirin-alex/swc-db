/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Pread.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Pread::Pread(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
             uint64_t offset, size_t len, 
             const FS::Callback::PreadCb_t& cb)
            : Base(Buffers::make(Params::PreadReq(smartfd->fd(), offset, len))), 
              smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_PREAD, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read offset=%lu len=%lu timeout=%d ", 
                    offset, len, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void Pread::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  size_t amount = 0;
  Base::handle_pread(ev, smartfd, amount);
  StaticBuffer::Ptr buf(amount ? new StaticBuffer(ev->data_ext) : nullptr);
  cb(error, smartfd, buf);
}



}}}}}
