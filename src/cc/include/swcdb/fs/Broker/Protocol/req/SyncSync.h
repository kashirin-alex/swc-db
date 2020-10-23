/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_SyncSync_h
#define swcdb_fs_Broker_Protocol_req_SyncSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class SyncSync : public BaseSync, public Base {
  public:
  
  SyncSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd)
          : Base(
              Buffers::make(
                Params::SyncReq(smartfd->fd()),
                0,
                FUNCTION_SYNC, timeout
              )
            ),
            smartfd(smartfd) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_sync(ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr& smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_SyncSync_h
