/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Flush_h
#define swcdb_fs_Broker_Protocol_req_Flush_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Flush.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Flush : public Base {
  public:

  Flush(uint32_t timeout, FS::SmartFd::Ptr& smartfd, 
        const FS::Callback::FlushCb_t& cb);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FS::SmartFd::Ptr                smartfd;
  const FS::Callback::FlushCb_t   cb;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Flush.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Flush_h
