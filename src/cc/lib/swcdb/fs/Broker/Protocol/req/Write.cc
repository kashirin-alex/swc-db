/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Write.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Write::Write(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
             uint8_t replication, int64_t blksz, StaticBuffer& buffer,
             const FS::Callback::WriteCb_t& cb) 
            : smartfd(smartfd), cb(cb) {
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

void Write::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_WRITE, &ptr, &remain))
    return;

  smartfd->fd(-1);
  smartfd->pos(0);

  SWC_LOG_OUT(LOG_DEBUG, 
    Error::print(SWC_LOG_OSTREAM << "write ", error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  cb(error, smartfd);
}



}}}}}
