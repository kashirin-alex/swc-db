/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_CloseSync_h
#define swcdb_fs_Broker_Protocol_req_CloseSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Close.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class CloseSync : public BaseSync, public Base {
  public:

  CloseSync(FS::FileSystem::Ptr fs, uint32_t timeout, 
            FS::SmartFd::Ptr& smartfd);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::FileSystem::Ptr    fs;
  FS::SmartFd::Ptr&      smartfd;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/CloseSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_CloseSync_h
