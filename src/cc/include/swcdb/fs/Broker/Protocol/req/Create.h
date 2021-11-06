/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Create_h
#define swcdb_fs_Broker_Protocol_req_Create_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Create final : public Base {
  public:
  typedef std::shared_ptr<Create> Ptr;

  SWC_CAN_INLINE
  Create(const FS::FileSystem::Ptr& a_fs,
         uint32_t timeout, FS::SmartFd::Ptr& a_smartfd,
         uint8_t replication, int64_t blksz,
         FS::Callback::CreateCb_t&& a_cb)
         : Base(
            a_fs->statistics, FS::Statistics::CREATE_ASYNC,
            Buffers::make(
              Params::CreateReq(
                a_smartfd->filepath(), a_smartfd->flags(),
                replication, blksz
              ),
              0,
              FUNCTION_CREATE, timeout
            )
          ),
          fs(a_fs), smartfd(a_smartfd), cb(std::move(a_cb)) {
  }

  ~Create() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_create(fs, ev, smartfd);
    cb(error, smartfd);
  }

  private:
  FS::FileSystem::Ptr             fs;
  FS::SmartFd::Ptr                smartfd;
  const FS::Callback::CreateCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Create_h
