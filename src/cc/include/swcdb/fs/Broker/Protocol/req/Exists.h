/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Exists_h
#define swcdb_fs_Broker_Protocol_req_Exists_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Exists : public Base {
  public:

  Exists(uint32_t timeout, const std::string& name, 
         const FS::Callback::ExistsCb_t& cb);

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string               name;
  const FS::Callback::ExistsCb_t  cb;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Exists.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Exists_h
