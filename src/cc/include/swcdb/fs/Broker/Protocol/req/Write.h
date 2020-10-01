/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Write_h
#define swcdb_fs_Broker_Protocol_req_Write_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Write.h"

namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {

class Write : public Base {

  public:
  
  Write(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
        uint8_t replication, int64_t blksz, StaticBuffer& buffer,
        const FS::Callback::WriteCb_t& cb=0);

  std::promise<void> promise();

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr         smartfd;
  FS::Callback::WriteCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Write.cc"
#endif 


#endif // swcdb_fs_Broker_Protocol_req_Write_h
