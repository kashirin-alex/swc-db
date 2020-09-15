/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Readdir_h
#define swc_fs_Broker_Protocol_req_Readdir_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Readdir.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Readdir : public Base {

  public:

  DirentList listing;

  Readdir(uint32_t timeout, const std::string& name, 
          const Callback::ReaddirCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  const std::string      name;
  Callback::ReaddirCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Readdir.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Readdir_h
