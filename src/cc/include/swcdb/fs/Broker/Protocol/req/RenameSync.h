/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_RenameSync_h
#define swcdb_fs_Broker_Protocol_req_RenameSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rename.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class RenameSync : public BaseSync, public Base {
  public:

  RenameSync(uint32_t timeout, 
             const std::string& from, const std::string& to);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string from;
  const std::string to;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/RenameSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_RenameSync_h
