/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_OpenSync_h
#define swcdb_fs_Broker_Protocol_req_OpenSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class OpenSync : public BaseSync, public Base {
  public:

  OpenSync(FS::FileSystem::Ptr fs,
           uint32_t timeout, FS::SmartFd::Ptr& smartfd,
           int32_t bufsz);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::FileSystem::Ptr     fs;
  FS::SmartFd::Ptr&       smartfd;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/OpenSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_OpenSync_h
