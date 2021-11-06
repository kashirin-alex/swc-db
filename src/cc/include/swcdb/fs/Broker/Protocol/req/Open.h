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
  typedef std::shared_ptr<Open> Ptr;

  SWC_CAN_INLINE
  Open(const FS::FileSystem::Ptr& a_fs,
       uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
       FS::Callback::OpenCb_t&& a_cb)
      : Base(
          a_fs->statistics, FS::Statistics::OPEN_ASYNC,
          Buffers::make(
            Params::OpenReq(a_smartfd->filepath(), a_smartfd->flags()),
            0,
            FUNCTION_OPEN, timeout
          )
        ),
        fs(a_fs), smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Open() noexcept { }

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
