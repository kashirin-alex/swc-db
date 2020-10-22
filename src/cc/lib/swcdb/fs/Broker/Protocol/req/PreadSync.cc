/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/PreadSync.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


PreadSync::PreadSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
                     uint64_t offset, void* dst, size_t len, bool allocated)
                    : Base(Buffers::make(Params::PreadReq(
                        smartfd->fd(), offset, len))), 
                      buffer(dst), allocated(allocated), amount(0), 
                      smartfd(smartfd) {
  cbp->header.set(FUNCTION_PREAD, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read offset=%lu len=%lu timeout=%d ", 
                    offset, len, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void PreadSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_PREAD, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
    case Error::FS_EOF: {
      Params::ReadRsp params;
      try {
        params.decode(&ptr, &remain);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
        break;
      }
      amount = ev->data_ext.size;
      smartfd->pos(params.offset + amount);
      if(allocated) {
        if(amount)
          memcpy(buffer, ev->data_ext.base, amount);
      } else {
        ((StaticBuffer*)buffer)->set(ev->data_ext);
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
    SWC_LOG_PRINTF("pread amount=%lu ", amount);
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );

  BaseSync::acknowledge();
}



}}}}}
