/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Remove_h
#define swcdb_fs_Broker_Protocol_req_Remove_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Remove final : public Base {
  public:

  Remove(FS::Statistics& stats,
         uint32_t timeout, const std::string& name,
         FS::Callback::RemoveCb_t&& cb)
        : Base(
            stats, FS::Statistics::REMOVE_ASYNC,
            Buffers::make(
              Params::RemoveReq(name),
              0,
              FUNCTION_REMOVE, timeout
            )
          ),
          name(name), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_remove(ev, name);
    cb(error);
  }

  private:
  const std::string               name;
  const FS::Callback::RemoveCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Remove_h
