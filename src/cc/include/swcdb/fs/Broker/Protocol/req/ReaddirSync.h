/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReaddirSync_h
#define swcdb_fs_Broker_Protocol_req_ReaddirSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReaddirSync : public BaseSync, public Base {
  public:

  FS::DirentList& listing;

  ReaddirSync(uint32_t timeout, const std::string& name, 
              FS::DirentList& listing);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string name;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/ReaddirSync.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_ReaddirSync_h
