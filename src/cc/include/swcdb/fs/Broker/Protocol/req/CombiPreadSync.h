/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_CombiPreadSync_h
#define swcdb_fs_Broker_Protocol_req_CombiPreadSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/CombiPread.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class CombiPreadSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<CombiPreadSync> Ptr;

  StaticBuffer* buffer;

  SWC_CAN_INLINE
  CombiPreadSync(FS::Statistics& stats,
                 uint32_t timeout, const FS::SmartFd::Ptr& a_smartfd,
                 uint64_t offset, uint32_t amount, StaticBuffer* dst)
                : Base(
                    stats, FS::Statistics::COMBI_PREAD_SYNC,
                    Buffers::make(
                      Params::CombiPreadReq(a_smartfd, offset, amount),
                      0,
                      FUNCTION_COMBI_PREAD, timeout
                    )
                  ),
                  buffer(dst), smartfd(a_smartfd) {
  }

  CombiPreadSync(CombiPreadSync&&)                 = delete;
  CombiPreadSync(const CombiPreadSync&)            = delete;
  CombiPreadSync& operator=(CombiPreadSync&&)      = delete;
  CombiPreadSync& operator=(const CombiPreadSync&) = delete;

  ~CombiPreadSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_combi_pread(ev, smartfd);
    if(!error)
      buffer->set(ev->data_ext);
    BaseSync::acknowledge();
  }

  private:
  const FS::SmartFd::Ptr smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_CombiPreadSync_h
