/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_CloseSync_h
#define swcdb_fs_Broker_Protocol_req_CloseSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Close.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class CloseSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<CloseSync> Ptr;

  SWC_CAN_INLINE
  CloseSync(const FS::FileSystem::Ptr& a_fs, uint32_t timeout,
            FS::SmartFd::Ptr& a_smartfd)
            : Base(
                a_fs->statistics, FS::Statistics::CLOSE_SYNC,
                Buffers::make(
                  Params::CloseReq(a_smartfd->fd()),
                  0,
                  FUNCTION_CLOSE, timeout
                )
              ),
              fs(a_fs), smartfd(a_smartfd) {
  }

  ~CloseSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_close(fs, ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::FileSystem::Ptr    fs;
  FS::SmartFd::Ptr&      smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_CloseSync_h
