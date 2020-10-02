/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Append_h
#define swcdb_fs_Broker_Protocol_req_Append_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {

class Append : public Base {

  public:
  
  size_t amount;
  
  Append(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
         StaticBuffer& buffer, FS::Flags flags, 
         const FS::Callback::AppendCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr          smartfd;
  FS::Callback::AppendCb_t  cb;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Append.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Append_h
