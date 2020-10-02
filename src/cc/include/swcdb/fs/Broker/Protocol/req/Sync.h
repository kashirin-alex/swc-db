/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Sync_h
#define swcdb_fs_Broker_Protocol_req_Sync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Sync.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {

class Sync : public Base {

  public:
  
  Sync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
       const FS::Callback::SyncCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr        smartfd;
  FS::Callback::SyncCb_t  cb;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Sync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Sync_h
