/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/OpenSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


OpenSync::OpenSync(FS::FileSystem::Ptr fs,
                   uint32_t timeout, FS::SmartFd::Ptr& smartfd,
                   int32_t bufsz)
                  : Base(Buffers::make(Params::OpenReq(
                      smartfd->filepath(), smartfd->flags(), bufsz))),
                    fs(fs), smartfd(smartfd) {
  cbp->header.set(FUNCTION_OPEN, timeout);
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("open timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM); 
  );
}

void OpenSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_open(fs, ev, smartfd);
  BaseSync::acknowledge();
}



}}}}}
