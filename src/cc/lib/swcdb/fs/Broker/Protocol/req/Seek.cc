/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Seek.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Seek::Seek(uint32_t timeout, SmartFd::Ptr& smartfd, size_t offset,
           const Callback::SeekCb_t& cb) 
          : smartfd(smartfd), cb(cb) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  cbp = CommBuf::make(Params::SeekReq(smartfd->fd(), offset));
  cbp->header.set(Cmd::FUNCTION_SEEK, timeout);
}

std::promise<void> Seek::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise] (int, const SmartFd::Ptr&){ await->set_value(); };
  return r_promise;
}

void Seek::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_SEEK, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      Params::SeekRsp params;
      try {
        params.decode(&ptr, &remain);
      } catch(...) {
        const Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
        break;
      }
      smartfd->pos(params.offset);
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
  
  cb(error, smartfd);
}



}}}}
