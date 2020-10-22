/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReadAllSync_h
#define swcdb_fs_Broker_Protocol_req_ReadAllSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReadAllSync : public BaseSync, public Base {
  public:

  StaticBuffer* buffer;
  
  ReadAllSync(uint32_t timeout, const std::string& name, StaticBuffer* dst);

  void handle(ConnHandlerPtr, const Event::Ptr& ev);

  private:
  const std::string                 name;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/ReadAllSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_ReadAllSync_h
