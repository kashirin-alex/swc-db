/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Append::Append(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
               StaticBuffer& buffer, FS::Flags flags, 
               const FS::Callback::AppendCb_t& cb)
              : smartfd(smartfd), cb(cb) {
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

void Append::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_APPEND, &ptr, &remain))
    return;

  Params::AppendRsp params;
  switch(error) {
    case Error::OK: {
      try {
        params.decode(&ptr, &remain);
        smartfd->pos(params.offset + params.amount);
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
    SWC_LOG_PRINTF("append amount=%u ", params.amount);
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  cb(error, smartfd, params.amount);
}



}}}}}
