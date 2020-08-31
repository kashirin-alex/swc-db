/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Write.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Write::Write(uint32_t timeout, SmartFd::Ptr& smartfd, 
             uint8_t replication, int64_t blksz, StaticBuffer& buffer,
             const Callback::WriteCb_t& cb) 
            : smartfd(smartfd), cb(cb) {
  SWC_LOGF(LOG_DEBUG, 
    "write amount=%lu %s replication(%u) blksz(%ld)", 
    buffer.size, smartfd->to_string().c_str(), replication, blksz);

  cbp = CommBuf::make(
    Params::WriteReq(smartfd->filepath(), smartfd->flags(), 
                     replication, blksz), 
    buffer
  );
  cbp->header.set(Cmd::FUNCTION_WRITE, timeout);
}

std::promise<void> Write::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, const SmartFd::Ptr&){ await->set_value(); };
  return r_promise;
}

void Write::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_WRITE, &ptr, &remain))
    return;

  smartfd->fd(-1);
  smartfd->pos(0);
  SWC_LOGF(LOG_DEBUG, "write %s error='%d'", 
            smartfd->to_string().c_str(), error);
  
  cb(error, smartfd);
}



}}}}
