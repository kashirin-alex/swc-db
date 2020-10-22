/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_WriteSync_h
#define swcdb_fs_Broker_Protocol_req_WriteSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Write.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class WriteSync : public BaseSync, public Base {

  public:
  
  WriteSync(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
            uint8_t replication, int64_t blksz, StaticBuffer& buffer);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr& smartfd;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/WriteSync.cc"
#endif 


#endif // swcdb_fs_Broker_Protocol_req_WriteSync_h
