/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Rename_h
#define swcdb_fs_Broker_Protocol_req_Rename_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rename.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Rename final : public Base {
  public:

  Rename(uint32_t timeout, const std::string& from, const std::string& to,
         FS::Callback::RenameCb_t&& cb)
        : Base(
            Buffers::make(
              Params::RenameReq(from, to),
              0,
              FUNCTION_RENAME, timeout
            )
          ),
          from(from), to(to), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_rename(ev, from, to);
    cb(error);
  }

  private:
  const std::string               from;
  const std::string               to;
  const FS::Callback::RenameCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Rename_h
