/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Create.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Create::Create(FS::FileSystem::Ptr fs,
               uint32_t timeout, FS::SmartFd::Ptr& smartfd,
               int32_t bufsz, uint8_t replication, int64_t blksz, 
               const FS::Callback::CreateCb_t& cb) 
              : Base(Buffers::make(Params::CreateReq(
                  smartfd->filepath(), smartfd->flags(),
                  bufsz, replication, blksz))),
                fs(fs), smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_CREATE, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("create bufsz(%d) replication(%d) blksz(%ld) timeout=%d ", 
                    bufsz, replication, blksz, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
}

void Create::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_create(fs, ev, smartfd);
  cb(error, smartfd);
}



}}}}}
