/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Create_h
#define swc_fs_Broker_Protocol_req_Create_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Create : public Base {

  public:
  
  Create(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr& smartfd, 
        int32_t bufsz, uint8_t replication, int64_t blksz, 
        const Callback::CreateCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FileSystem::Ptr       fs;
  SmartFd::Ptr          smartfd;
  Callback::CreateCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Create.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Create_h
