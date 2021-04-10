/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Sync_h
#define swcdb_fs_Broker_Protocol_req_Sync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Sync final : public Base {
  public:

  Sync(uint32_t timeout, FS::SmartFd::Ptr& smartfd,
       FS::Callback::SyncCb_t&& cb)
       : Base(
          Buffers::make(
            Params::SyncReq(smartfd->fd()),
            0,
            FUNCTION_SYNC, timeout
          )
        ),
        smartfd(smartfd), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_sync(ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::SyncCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Sync_h
