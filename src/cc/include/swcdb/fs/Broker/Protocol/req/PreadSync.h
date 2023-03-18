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
  typedef std::shared_ptr<PreadSync> Ptr;

  void*   buffer;
  bool    allocated;
  size_t  amount;

  SWC_CAN_INLINE
  PreadSync(FS::Statistics& stats,
            uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
            uint64_t offset, void* dst, size_t len, bool a_allocated)
            : Base(
                stats, FS::Statistics::PREAD_SYNC,
                Buffers::make(
                  Params::PreadReq(a_smartfd->fd(), offset, len),
                  0,
                  FUNCTION_PREAD, timeout
                )
              ),
              buffer(dst), allocated(a_allocated), amount(0),
              smartfd(a_smartfd) {
  }

  PreadSync(PreadSync&&)                 = delete;
  PreadSync(const PreadSync&)            = delete;
  PreadSync& operator=(PreadSync&&)      = delete;
  PreadSync& operator=(const PreadSync&) = delete;

  ~PreadSync() noexcept { }

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
