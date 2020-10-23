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


class WriteSync : public BaseSync, public Base {

  public:
  
  WriteSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
            uint8_t replication, int64_t blksz, StaticBuffer& buffer)
            : Base(
                Buffers::make(
                  Params::WriteReq(
                    smartfd->filepath(), smartfd->flags(),
                    replication, blksz
                  ),
                  buffer,
                  0,
                  FUNCTION_WRITE, timeout
                )
              ),
              smartfd(smartfd) {
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
