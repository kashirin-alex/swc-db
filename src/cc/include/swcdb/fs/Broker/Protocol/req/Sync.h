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
  typedef std::shared_ptr<Sync> Ptr;

  SWC_CAN_INLINE
  Sync(FS::Statistics& stats,
       uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
       FS::Callback::SyncCb_t&& a_cb)
       : Base(
          stats, FS::Statistics::SYNC_ASYNC,
          Buffers::make(
            Params::SyncReq(a_smartfd->fd()),
            0,
            FUNCTION_SYNC, timeout
          )
        ),
        smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Sync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_sync(ev, smartfd);
    cb(error);
  }

  private:
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::SyncCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Sync_h
