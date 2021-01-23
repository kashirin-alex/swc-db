/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Open_h
#define swcdb_fs_Broker_Protocol_req_Open_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Open final : public Base {
  public:

  Open(const FS::FileSystem::Ptr& fs,
       uint32_t timeout, FS::SmartFd::Ptr& smartfd, int32_t bufsz,
       const FS::Callback::OpenCb_t& cb)
      : Base(
          Buffers::make(
            Params::OpenReq(smartfd->filepath(), smartfd->flags(), bufsz),
            0,
            FUNCTION_OPEN, timeout
          )
        ),
        fs(fs), smartfd(smartfd), cb(cb) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_open(fs, ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::FileSystem::Ptr           fs;
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::OpenCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Open_h
