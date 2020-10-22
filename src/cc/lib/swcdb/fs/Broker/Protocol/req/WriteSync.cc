/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/WriteSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


WriteSync::WriteSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
                     uint8_t replication, int64_t blksz, StaticBuffer& buffer)
                    : smartfd(smartfd) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("write amount=%lu replication(%u) blksz(%ld) timeout=%d ",
                    buffer.size, replication, blksz, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  cbp = Buffers::make(
    Params::WriteReq(smartfd->filepath(), smartfd->flags(), 
                     replication, blksz), 
    buffer
  );
  cbp->header.set(FUNCTION_WRITE, timeout);
}


void WriteSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_write(ev, smartfd);
  BaseSync::acknowledge();
}



}}}}}
