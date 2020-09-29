/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Rmdir_h
#define swc_fs_Broker_Protocol_req_Rmdir_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Rmdir : public Base {

  public:

  Rmdir(uint32_t timeout, const std::string& name, 
        const Callback::RmdirCb_t& cb=0);

  std::promise<void> promise();

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override;

  private:
  const std::string    name;
  Callback::RmdirCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Rmdir.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Rmdir_h
