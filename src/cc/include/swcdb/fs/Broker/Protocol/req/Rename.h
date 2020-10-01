/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Rename_h
#define swcdb_fs_Broker_Protocol_req_Rename_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rename.h"

namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {

class Rename : public Base {

  public:

  Rename(uint32_t timeout, const std::string& from, const std::string& to,
        const FS::Callback::RenameCb_t& cb=0);

  std::promise<void> promise();

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override;

  private:
  const std::string         from;
  const std::string         to;
  FS::Callback::RenameCb_t  cb;

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Rename.cc"
#endif 

#endif // swcdb_fs_Broker_Protocol_req_Rename_h
