/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/SyncSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


SyncSync::SyncSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd)
                  : Base(Buffers::make(Params::SyncReq(smartfd->fd()))),
                    smartfd(smartfd) {
  cbp->header.set(FUNCTION_SYNC, timeout);
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("sync timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}


void SyncSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_SYNC, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
      break;
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  
  SWC_LOG_OUT(LOG_DEBUG, 
    Error::print(SWC_LOG_OSTREAM << "sync ", error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  BaseSync::acknowledge();
}



}}}}}
