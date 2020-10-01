/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Length_h
#define swcdb_fs_Broker_Protocol_req_Length_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Length.h"

namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {

class Length : public Base {

  public:

  size_t length;

  Length(uint32_t timeout, const std::string& name, 
         const FS::Callback::LengthCb_t& cb=0);

  std::promise<void> promise();

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override;

  private:
  const std::string         name;
  FS::Callback::LengthCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Length.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Length_h
