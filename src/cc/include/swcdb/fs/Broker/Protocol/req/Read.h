/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Read_h
#define swcdb_fs_Broker_Protocol_req_Read_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Read final : public Base {
  public:
  typedef std::shared_ptr<Read> Ptr;

  SWC_CAN_INLINE
  Read(FS::Statistics& stats,
       uint32_t timeout, FS::SmartFd::Ptr& a_smartfd, size_t len,
       FS::Callback::ReadCb_t&& a_cb)
      : Base(
          stats, FS::Statistics::READ_ASYNC,
          Buffers::make(
            Params::ReadReq(a_smartfd->fd(), len),
            0,
            FUNCTION_READ, timeout
          )
        ),
        smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Read() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    size_t amount = 0;
    Base::handle_read(ev, smartfd, amount);
    cb(
      error,
      std::move(ev->data_ext)
    );
  }

  private:
  FS::SmartFd::Ptr              smartfd;
  const FS::Callback::ReadCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Read_h
