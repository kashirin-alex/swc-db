/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Write.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Write::Write(uint32_t timeout, SmartFd::Ptr& smartfd, 
             uint8_t replication, int64_t blksz, StaticBuffer& buffer,
             const Callback::WriteCb_t& cb) 
            : smartfd(smartfd), cb(cb) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("write amount=%lu replication(%u) blksz(%ld) timeout=%d ",
                    buffer.size, replication, blksz, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  cbp = Comm::CommBuf::make(
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

void Write::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_WRITE, &ptr, &remain))
    return;

  smartfd->fd(-1);
  smartfd->pos(0);

  SWC_LOG_OUT(LOG_DEBUG, 
    Error::print(SWC_LOG_OSTREAM << "write ", error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  cb(error, smartfd);
}



}}}}
