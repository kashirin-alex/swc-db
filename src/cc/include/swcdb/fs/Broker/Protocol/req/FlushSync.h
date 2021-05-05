/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_FlushSync_h
#define swcdb_fs_Broker_Protocol_req_FlushSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Flush.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class FlushSync final : public BaseSync, public Base {
  public:

  FlushSync(FS::Statistics& stats,
            uint32_t timeout, FS::SmartFd::Ptr& smartfd)
            : Base(
                stats, FS::Statistics::FLUSH_SYNC,
                Buffers::make(
                  Params::FlushReq(smartfd->fd()),
                  0,
                  FUNCTION_FLUSH, timeout
                )
              ),
              smartfd(smartfd) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_flush(ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr&  smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_FlushSync_h
