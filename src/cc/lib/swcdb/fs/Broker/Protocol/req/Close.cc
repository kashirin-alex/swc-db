/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Close.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Close::Close(FS::FileSystem::Ptr fs, uint32_t timeout, 
             FS::SmartFd::Ptr& smartfd,
             const FS::Callback::CloseCb_t& cb)
            : Base(Buffers::make(Params::CloseReq(smartfd->fd()))),
              fs(fs), smartfd(smartfd), cb(cb) {
  cbp->header.set(FUNCTION_CLOSE, timeout);
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("close timeout=%d ", timeout);
    smartfd->print(SWC_LOG_OSTREAM); 
  );
}

void Close::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_close(fs, ev, smartfd);
  cb(error, smartfd);
}



}}}}}
