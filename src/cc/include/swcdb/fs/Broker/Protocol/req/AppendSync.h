/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_AppendSync_h
#define swcdb_fs_Broker_Protocol_req_AppendSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class AppendSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<AppendSync> Ptr;

  size_t amount;

  SWC_CAN_INLINE
  AppendSync(FS::Statistics& stats,
             uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
             StaticBuffer& buffer, FS::Flags flags)
        : Base(
            stats, FS::Statistics::APPEND_SYNC,
            Buffers::make(
              Params::AppendReq(a_smartfd->fd(), flags),
              buffer,
              0,
              FUNCTION_APPEND, timeout
            )
          ),
          amount(0), smartfd(a_smartfd) {
  }

  ~AppendSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_append(ev, smartfd, amount);
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr& smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_AppendSync_h
