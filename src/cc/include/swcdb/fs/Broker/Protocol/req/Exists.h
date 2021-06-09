/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Exists_h
#define swcdb_fs_Broker_Protocol_req_Exists_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Exists final : public Base {
  public:
  typedef std::shared_ptr<Exists> Ptr;

  SWC_CAN_INLINE
  Exists(FS::Statistics& stats,
         uint32_t timeout, const std::string& name,
         FS::Callback::ExistsCb_t&& cb)
        : Base(
            stats, FS::Statistics::EXISTS_ASYNC,
            Buffers::make(
              Params::ExistsReq(name),
              0,
              FUNCTION_EXISTS, timeout
            )
          ),
          name(name), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    bool state = false;
    Base::handle_exists(ev, name, state);
    cb(error, state);
  }


  private:
  const std::string               name;
  const FS::Callback::ExistsCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Exists_h
