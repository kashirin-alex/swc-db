/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Exists_h
#define swc_fs_Broker_Protocol_req_Exists_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Exists.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Exists : public Base {

  public:

  bool  state;

  Exists(uint32_t timeout, const std::string& name, 
         const Callback::ExistsCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string     name;
  Callback::ExistsCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Exists.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Exists_h
