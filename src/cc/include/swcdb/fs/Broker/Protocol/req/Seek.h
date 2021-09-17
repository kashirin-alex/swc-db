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
  typedef std::shared_ptr<Seek> Ptr;

  SWC_CAN_INLINE
  Seek(FS::Statistics& stats,
       uint32_t timeout, FS::SmartFd::Ptr& a_smartfd, size_t offset,
       FS::Callback::SeekCb_t&& a_cb)
      : Base(
         stats, FS::Statistics::SEEK_ASYNC,
          Buffers::make(
            Params::SeekReq(a_smartfd->fd(), offset),
            0,
            FUNCTION_SEEK, timeout
          )
        ),
        smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Seek() noexcept { }

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
