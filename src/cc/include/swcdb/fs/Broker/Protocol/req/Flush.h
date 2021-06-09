/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Flush_h
#define swcdb_fs_Broker_Protocol_req_Flush_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Flush.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Flush final : public Base {
  public:
  typedef std::shared_ptr<Flush> Ptr;

  SWC_CAN_INLINE
  Flush(FS::Statistics& stats,
        uint32_t timeout, FS::SmartFd::Ptr& smartfd,
        FS::Callback::FlushCb_t&& cb)
        : Base(
            stats, FS::Statistics::FLUSH_ASYNC,
            Buffers::make(
              Params::FlushReq(smartfd->fd()),
              0,
              FUNCTION_FLUSH, timeout
            )
          ),
          smartfd(smartfd), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_flush(ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::SmartFd::Ptr                smartfd;
  const FS::Callback::FlushCb_t   cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Flush_h
