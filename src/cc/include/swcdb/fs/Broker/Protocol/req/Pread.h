/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Pread_h
#define swcdb_fs_Broker_Protocol_req_Pread_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Pread final : public Base {
  public:
  typedef std::shared_ptr<Pread> Ptr;

  SWC_CAN_INLINE
  Pread(FS::Statistics& stats,
        uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
        uint64_t offset, size_t len,
        FS::Callback::PreadCb_t&& a_cb)
        : Base(
            stats, FS::Statistics::PREAD_ASYNC,
            Buffers::make(
              Params::PreadReq(a_smartfd->fd(), offset, len),
              0,
              FUNCTION_PREAD, timeout
            )
          ),
          smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Pread() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    size_t amount = 0;
    Base::handle_pread(ev, smartfd, amount);
    cb(
      error,
      std::move(ev->data_ext)
    );
  }

  private:
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::PreadCb_t cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Pread_h
