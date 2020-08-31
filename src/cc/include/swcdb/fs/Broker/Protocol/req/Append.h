/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Append_h
#define swc_fs_Broker_Protocol_req_Append_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Append : public Base {

  public:
  
  size_t amount;
  
  Append(uint32_t timeout, SmartFd::Ptr& smartfd, 
         StaticBuffer& buffer, Flags flags, const Callback::AppendCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  SmartFd::Ptr          smartfd;
  Callback::AppendCb_t  cb;

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Append.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Append_h
