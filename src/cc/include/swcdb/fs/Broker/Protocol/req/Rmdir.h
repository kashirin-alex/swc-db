/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Rmdir_h
#define swcdb_fs_Broker_Protocol_req_Rmdir_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Rmdir final : public Base {
  public:
  typedef std::shared_ptr<Rmdir> Ptr;

  SWC_CAN_INLINE
  Rmdir(FS::Statistics& stats,
        uint32_t timeout, const std::string& name,
        FS::Callback::RmdirCb_t&& cb)
        : Base(
            stats, FS::Statistics::RMDIR_ASYNC,
            Buffers::make(
              Params::RmdirReq(name),
              0,
              FUNCTION_RMDIR, timeout
            )
          ),
          name(name), cb(std::move(cb)) {
}

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_rmdir(ev, name);
    cb(error);
  }

  private:
  const std::string               name;
  const FS::Callback::RmdirCb_t   cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Rmdir_h
