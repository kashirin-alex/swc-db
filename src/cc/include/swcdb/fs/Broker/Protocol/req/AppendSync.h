/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_AppendSync_h
#define swcdb_fs_Broker_Protocol_req_AppendSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class AppendSync :  public BaseSync, public Base {
  public:
  
  size_t amount;
  
  AppendSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
             StaticBuffer& buffer, FS::Flags flags);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr& smartfd;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/AppendSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_AppendSync_h
