/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_CreateSync_h
#define swcdb_fs_Broker_Protocol_req_CreateSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class CreateSync final : public BaseSync, public Base {
  public:

  CreateSync(const FS::FileSystem::Ptr& fs,
             uint32_t timeout, FS::SmartFd::Ptr& smartfd,
             int32_t bufsz, uint8_t replication, int64_t blksz)
            : Base(
                fs->statistics, FS::Statistics::CREATE_SYNC,
                Buffers::make(
                  Params::CreateReq(
                    smartfd->filepath(), smartfd->flags(),
                    bufsz, replication, blksz
                  ),
                  0,
                  FUNCTION_CREATE, timeout
                )
              ),
              fs(fs), smartfd(smartfd) {
}


  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_create(fs, ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::FileSystem::Ptr  fs;
  FS::SmartFd::Ptr&    smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_CreateSync_h
