/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/CreateSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


CreateSync::CreateSync(FS::FileSystem::Ptr fs,
                       uint32_t timeout, FS::SmartFd::Ptr& smartfd,
                       int32_t bufsz, uint8_t replication, int64_t blksz)
                      : Base(Buffers::make(Params::CreateReq(
                          smartfd->filepath(), smartfd->flags(),
                          bufsz, replication, blksz))),
                        fs(fs), smartfd(smartfd) {
  cbp->header.set(FUNCTION_CREATE, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("create bufsz(%d) replication(%d) blksz(%ld) timeout=%d ", 
                    bufsz, replication, blksz, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void CreateSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_CREATE, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("create fds-open=%lu ", fs->fds_open());
    Error::print(SWC_LOG_OSTREAM, error);
    smartfd->print(SWC_LOG_OSTREAM << ' ');
  );
  
  BaseSync::acknowledge();
}



}}}}}
