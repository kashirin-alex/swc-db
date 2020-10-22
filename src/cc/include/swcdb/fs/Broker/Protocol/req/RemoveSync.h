/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_RemoveSync_h
#define swcdb_fs_Broker_Protocol_req_RemoveSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class RemoveSync : public BaseSync, public Base {
  public:

  RemoveSync(uint32_t timeout, const std::string& name);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string name;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/RemoveSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_RemoveSync_h