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

  CloseSync(const FS::FileSystem::Ptr& fs, uint32_t timeout,
            FS::SmartFd::Ptr& smartfd)
            : Base(
                Buffers::make(
                  Params::CloseReq(smartfd->fd()),
                  0,
                  FUNCTION_CLOSE, timeout
                )
              ),
              fs(fs), smartfd(smartfd) {
}

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
