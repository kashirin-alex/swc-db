/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Read::Read(uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t len,
           const FS::Callback::ReadCb_t& cb)
          : Base(Buffers::make(Params::ReadReq(smartfd->fd(), len))), 
            smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_READ, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read len=%lu timeout=%d ", len, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void Read::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  size_t amount = 0;
  Base::handle_read(ev, smartfd, amount);
  StaticBuffer::Ptr buf(amount ? new StaticBuffer(ev->data_ext) : nullptr);
  cb(error, smartfd, buf);
}



}}}}}
