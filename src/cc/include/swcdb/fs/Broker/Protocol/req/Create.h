/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Create_h
#define swcdb_fs_Broker_Protocol_req_Create_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {

class Create : public Base {

  public:
  
  Create(FS::FileSystem::Ptr fs, uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
        int32_t bufsz, uint8_t replication, int64_t blksz, 
        const FS::Callback::CreateCb_t& cb=0);

  std::promise<void> promise();

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override;

  private:
  FS::FileSystem::Ptr       fs;
  FS::SmartFd::Ptr          smartfd;
  FS::Callback::CreateCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Create.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Create_h
