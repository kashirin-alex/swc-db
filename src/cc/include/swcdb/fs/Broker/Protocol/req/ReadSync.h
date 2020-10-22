/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReadSync_h
#define swcdb_fs_Broker_Protocol_req_ReadSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReadSync : public BaseSync, public Base {
  public:

  void*   buffer;
  bool    allocated;
  size_t  amount;

  ReadSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
           void* dst, size_t len, bool allocated);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr& smartfd;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/ReadSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_ReadSync_h
