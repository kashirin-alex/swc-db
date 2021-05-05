/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_PreadSync_h
#define swcdb_fs_Broker_Protocol_req_PreadSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class PreadSync final : public BaseSync, public Base {
  public:

  void*   buffer;
  bool    allocated;
  size_t  amount;

  PreadSync(FS::Statistics& stats,
            uint32_t timeout, FS::SmartFd::Ptr& smartfd,
            uint64_t offset, void* dst, size_t len, bool allocated)
            : Base(
                stats, FS::Statistics::PREAD_SYNC,
                Buffers::make(
                  Params::PreadReq(smartfd->fd(), offset, len),
                  0,
                  FUNCTION_PREAD, timeout
                )
              ),
              buffer(dst), allocated(allocated), amount(0), smartfd(smartfd) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_pread(ev, smartfd, amount);
    if(amount) {
      if(allocated) {
        memcpy(buffer, ev->data_ext.base, amount);
      } else {
        static_cast<StaticBuffer*>(buffer)->set(ev->data_ext);
      }
    }
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr& smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_PreadSync_h
