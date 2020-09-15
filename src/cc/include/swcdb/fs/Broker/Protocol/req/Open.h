/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Open_h
#define swc_fs_Broker_Protocol_req_Open_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Open : public Base {

  public:

  Open(FileSystem::Ptr fs, uint32_t timeout, SmartFd::Ptr& smartfd, 
       int32_t bufsz, const Callback::OpenCb_t& cb=0);

  std::promise<void> promise();

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  FileSystem::Ptr     fs;
  SmartFd::Ptr        smartfd;
  Callback::OpenCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Open.cc"
#endif 

#endif  // swc_fs_Broker_Protocol_req_Open_h
