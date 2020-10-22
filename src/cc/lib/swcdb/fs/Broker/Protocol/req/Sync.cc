/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Sync::Sync(uint32_t timeout, FS::SmartFd::Ptr& smartfd,
           const FS::Callback::SyncCb_t& cb)
          : Base(Buffers::make(Params::SyncReq(smartfd->fd()))),
            smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_SYNC, timeout);
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("sync timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void Sync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 
  Base::handle_sync(ev, smartfd);
  cb(error, smartfd);
}



}}}}}
