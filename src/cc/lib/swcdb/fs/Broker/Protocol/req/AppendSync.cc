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

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_APPEND, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      try {
        Params::AppendRsp params;
        params.decode(&ptr, &remain);
        amount = params.amount;
        smartfd->pos(params.offset + amount);
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
    SWC_LOG_PRINTF("append amount=%lu ", amount);
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  BaseSync::acknowledge();
}



}}}}}
