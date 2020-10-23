/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Write_h
#define swcdb_fs_Broker_Protocol_req_Write_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Write.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Write : public Base {
  public:
  
  Write(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
        uint8_t replication, int64_t blksz, StaticBuffer& buffer,
        const FS::Callback::WriteCb_t& cb)
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
          smartfd(smartfd), cb(cb) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_write(ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::SmartFd::Ptr               smartfd;
  const FS::Callback::WriteCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Write_h
