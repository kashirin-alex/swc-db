/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Write_h
#define swc_fs_Broker_Protocol_req_Write_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Write.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Write : public Base {

  public:
  
  Write(uint32_t timeout, SmartFd::Ptr& smartfd, 
        uint8_t replication, int64_t blksz, StaticBuffer& buffer,
        const Callback::WriteCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  SmartFd::Ptr         smartfd;
  Callback::WriteCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Write.cc"
#endif 


#endif  // swc_fs_Broker_Protocol_req_Write_h
