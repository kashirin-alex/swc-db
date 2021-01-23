/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Close_h
#define swcdb_fs_Broker_Protocol_req_Close_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Close.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Close final : public Base {
  public:

  Close(const FS::FileSystem::Ptr& fs, uint32_t timeout,
        FS::SmartFd::Ptr& smartfd,
        const FS::Callback::CloseCb_t& cb)
        : Base(
            Buffers::make(
              Params::CloseReq(smartfd->fd()),
              0,
              FUNCTION_CLOSE, timeout
            )
          ),
          fs(fs), smartfd(smartfd), cb(cb) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_close(fs, ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::FileSystem::Ptr             fs;
  FS::SmartFd::Ptr                smartfd;
  const FS::Callback::CloseCb_t   cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Close_h
