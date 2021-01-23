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

  Append(uint32_t timeout, FS::SmartFd::Ptr& smartfd,
         StaticBuffer& buffer, FS::Flags flags,
         const FS::Callback::AppendCb_t& cb)
        : Base(
            Buffers::make(
              Params::AppendReq(smartfd->fd(), (uint8_t)flags),
              buffer,
              0,
              FUNCTION_APPEND, timeout
            )
          ),
          smartfd(smartfd), cb(cb) {
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
