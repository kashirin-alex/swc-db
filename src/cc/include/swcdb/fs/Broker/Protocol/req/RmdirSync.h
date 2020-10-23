/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_RmdirSync_h
#define swcdb_fs_Broker_Protocol_req_RmdirSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class RmdirSync : public BaseSync, public Base {
  public:

  RmdirSync(uint32_t timeout, const std::string& name)
            : Base(
                Buffers::make(
                  Params::RmdirReq(name),
                  0,
                  FUNCTION_RMDIR, timeout
                )
              ),
              name(name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_rmdir(ev, name);
    BaseSync::acknowledge();
  }

  private:
  const std::string name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_RmdirSync_h
