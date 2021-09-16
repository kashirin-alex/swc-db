/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_WriteSync_h
#define swcdb_fs_Broker_Protocol_req_WriteSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Write.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class WriteSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<WriteSync> Ptr;

  SWC_CAN_INLINE
  WriteSync(FS::Statistics& stats,
            uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
            uint8_t replication, int64_t blksz, StaticBuffer& buffer)
            : Base(
                stats, FS::Statistics::WRITE_SYNC,
                Buffers::make(
                  Params::WriteReq(
                    a_smartfd->filepath(), a_smartfd->flags(),
                    replication, blksz
                  ),
                  buffer,
                  0,
                  FUNCTION_WRITE, timeout
                )
              ),
              smartfd(a_smartfd) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_write(ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr& smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_WriteSync_h
