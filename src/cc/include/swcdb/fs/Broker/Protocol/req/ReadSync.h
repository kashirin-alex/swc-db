/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReadSync_h
#define swcdb_fs_Broker_Protocol_req_ReadSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReadSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<ReadSync> Ptr;

  void*   buffer;
  bool    allocated;
  size_t  amount;

  SWC_CAN_INLINE
  ReadSync(FS::Statistics& stats,
           uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
           void* dst, size_t len, bool a_allocated)
          : Base(
              stats, FS::Statistics::READ_SYNC,
              Buffers::make(
                Params::ReadReq(a_smartfd->fd(), len),
                0,
                FUNCTION_READ, timeout
              )
            ),
            buffer(dst), allocated(a_allocated), amount(0),
            smartfd(a_smartfd) {
  }

  ReadSync(ReadSync&&)                 = delete;
  ReadSync(const ReadSync&)            = delete;
  ReadSync& operator=(ReadSync&&)      = delete;
  ReadSync& operator=(const ReadSync&) = delete;

  ~ReadSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_read(ev, smartfd, amount);
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


#endif // swcdb_fs_Broker_Protocol_req_ReadSync_h
