/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Mkdirs_h
#define swc_fs_Broker_Protocol_req_Mkdirs_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Mkdirs : public Base {

  public:

  Mkdirs(uint32_t timeout, const std::string& name, 
         const Callback::MkdirsCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string     name;
  Callback::MkdirsCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Mkdirs.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Mkdirs_h
