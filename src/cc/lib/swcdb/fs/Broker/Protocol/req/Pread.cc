/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Pread.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Pread::Pread(uint32_t timeout, SmartFd::Ptr& smartfd, 
      uint64_t offset, void* dst, size_t len, bool allocated,
      const Callback::PreadCb_t& cb)
    : buffer(dst), allocated(allocated), amount(0), 
      smartfd(smartfd), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "pread offset=%lu len=%lu timeout=%d %s", 
            offset, len, timeout, smartfd->to_string().c_str());
  cbp = CommBuf::make(Params::PreadReq(smartfd->fd(), offset, len));
  cbp->header.set(Cmd::FUNCTION_PREAD, timeout);
}

std::promise<void> Pread::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise]
       (int, const SmartFd::Ptr&, const StaticBuffer::Ptr&) {
         await->set_value();
        };
  return r_promise;
}

void Pread::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_PREAD, &ptr, &remain))
    return;

  StaticBuffer::Ptr buf = nullptr;
  switch(error) {
    case Error::OK:
    case Error::FS_EOF: {
      Params::ReadRsp params;
      try {
        params.decode(&ptr, &remain);
      } catch(...) {
        const Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
        break;
      }
      amount = ev->data_ext.size;
      smartfd->pos(params.offset + amount);
      if(amount) {
        if(buffer == nullptr) {
          buf = std::make_shared<StaticBuffer>(ev->data_ext);
        } else {
          if(allocated)
            memcpy(buffer, ev->data_ext.base, amount);
          else
            ((StaticBuffer*)buffer)->set(ev->data_ext);
        }
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }

  SWC_LOGF(LOG_DEBUG, "pread %s amount='%lu' error='%d'", 
            smartfd->to_string().c_str(), amount, error);

  cb(error, smartfd, buf);
}



}}}}
