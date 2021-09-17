/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_OpenSync_h
#define swcdb_fs_Broker_Protocol_req_OpenSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class OpenSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<OpenSync> Ptr;

  SWC_CAN_INLINE
  OpenSync(const FS::FileSystem::Ptr& a_fs,
           uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
           int32_t bufsz)
          : Base(
              a_fs->statistics, FS::Statistics::OPEN_SYNC,
              Buffers::make(
                Params::OpenReq(
                  a_smartfd->filepath(), a_smartfd->flags(), bufsz),
                0,
                FUNCTION_OPEN, timeout
              )
            ),
            fs(a_fs), smartfd(a_smartfd) {
  }

  ~OpenSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_open(fs, ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::FileSystem::Ptr     fs;
  FS::SmartFd::Ptr&       smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_OpenSync_h
