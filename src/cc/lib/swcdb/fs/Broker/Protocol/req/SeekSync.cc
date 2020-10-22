/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/SeekSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


SeekSync::SeekSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t offset)
                  : Base(Buffers::make(Params::SeekReq(smartfd->fd(), offset))),
                    smartfd(smartfd) {
  cbp->header.set(FUNCTION_SEEK, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void SeekSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_SEEK, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      try {
        Params::SeekRsp params;
        params.decode(&ptr, &remain);
        smartfd->pos(params.offset);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    Error::print(SWC_LOG_OSTREAM << "seek ", error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  BaseSync::acknowledge();
}



}}}}}
