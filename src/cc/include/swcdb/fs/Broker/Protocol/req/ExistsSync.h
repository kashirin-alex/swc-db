/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ExistsSync_h
#define swcdb_fs_Broker_Protocol_req_ExistsSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ExistsSync : public BaseSync, public Base {
  public:

  bool state;

  ExistsSync(uint32_t timeout, const std::string& name)
            : Base(
                Buffers::make(
                  Params::ExistsReq(name),
                  0,
                  FUNCTION_EXISTS, timeout
                )
              ),
              state(false), name(name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_exists(ev, name, state);
    BaseSync::acknowledge();
  }

  private:
  const std::string  name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_ExistsSync_h
