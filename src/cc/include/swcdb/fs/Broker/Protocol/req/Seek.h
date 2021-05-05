/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Seek_h
#define swcdb_fs_Broker_Protocol_req_Seek_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Seek final : public Base {
  public:

  Seek(FS::Statistics& stats,
       uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t offset,
       FS::Callback::SeekCb_t&& cb)
      : Base(
         stats, FS::Statistics::SEEK_ASYNC,
          Buffers::make(
            Params::SeekReq(smartfd->fd(), offset),
            0,
            FUNCTION_SEEK, timeout
          )
        ),
        smartfd(smartfd), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_seek(ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::SeekCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Seek_h
