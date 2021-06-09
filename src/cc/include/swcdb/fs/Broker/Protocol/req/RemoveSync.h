/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_RemoveSync_h
#define swcdb_fs_Broker_Protocol_req_RemoveSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class RemoveSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<RemoveSync> Ptr;

  SWC_CAN_INLINE
  RemoveSync(FS::Statistics& stats,
             uint32_t timeout, const std::string& name)
            : Base(
                stats, FS::Statistics::REMOVE_SYNC,
                Buffers::make(
                  Params::RemoveReq(name),
                  0,
                  FUNCTION_REMOVE, timeout
                )
              ),
              name(name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_remove(ev, name);
    BaseSync::acknowledge();
  }

  private:
  const std::string name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_RemoveSync_h
