/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_CombiPread_h
#define swcdb_fs_Broker_Protocol_req_CombiPread_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/CombiPread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class CombiPread final : public Base {
  public:
  typedef std::shared_ptr<CombiPread> Ptr;

  SWC_CAN_INLINE
  CombiPread(FS::Statistics& stats,
             uint32_t timeout, const FS::SmartFd::Ptr& a_smartfd,
             uint64_t offset, uint32_t amount,
             FS::Callback::CombiPreadCb_t&& a_cb)
            : Base(
                stats, FS::Statistics::COMBI_PREAD_ASYNC,
                Buffers::make(
                  Params::CombiPreadReq(a_smartfd, offset, amount),
                  0,
                  FUNCTION_COMBI_PREAD, timeout
                )
              ),
              smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~CombiPread() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_combi_pread(ev, smartfd);
    StaticBuffer::Ptr buf(error ? nullptr : new StaticBuffer(ev->data_ext));
    cb(error, buf);
  }

  private:
  const FS::SmartFd::Ptr              smartfd;
  const FS::Callback::CombiPreadCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_CombiPread_h
