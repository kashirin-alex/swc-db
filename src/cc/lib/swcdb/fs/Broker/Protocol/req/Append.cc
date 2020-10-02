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
              : amount(0), smartfd(smartfd), cb(cb) {
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

std::promise<void> Append::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise]
       (int, const FS::SmartFd::Ptr&, size_t){ await->set_value(); };
  return r_promise;
}

void Append::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_APPEND, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      Params::AppendRsp params;
      try {
        params.decode(&ptr, &remain);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
        break;
      }
      amount = params.amount;
      smartfd->pos(params.offset + amount);
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
  
  cb(error, smartfd, amount);
}



}}}}}
