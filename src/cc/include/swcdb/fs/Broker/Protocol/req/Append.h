/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Append_h
#define swcdb_fs_Broker_Protocol_req_Append_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Append final : public Base {
  public:
  typedef std::shared_ptr<Append> Ptr;

  SWC_CAN_INLINE
  Append(FS::Statistics& stats,
         uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
         StaticBuffer& buffer, FS::Flags flags,
         FS::Callback::AppendCb_t&& a_cb)
        : Base(
            stats, FS::Statistics::APPEND_ASYNC,
            Buffers::make(
              Params::AppendReq(a_smartfd->fd(), flags),
              buffer,
              0,
              FUNCTION_APPEND, timeout
            )
          ),
          smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    size_t amount = 0;
    Base::handle_append(ev, smartfd, amount);
    cb(error, smartfd, amount);
  }

  private:
  FS::SmartFd::Ptr                smartfd;
  const FS::Callback::AppendCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Append_h
