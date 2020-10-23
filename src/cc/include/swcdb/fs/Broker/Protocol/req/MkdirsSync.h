/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_MkdirsSync_h
#define swcdb_fs_Broker_Protocol_req_MkdirsSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class MkdirsSync : public BaseSync, public Base {
  public:

  MkdirsSync(uint32_t timeout, const std::string& name)
            : Base(
                Buffers::make(
                  Params::MkdirsReq(name),
                  0,
                  FUNCTION_MKDIRS, timeout
                )
              ),
              name(name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_mkdirs(ev, name);
    BaseSync::acknowledge();
  }

  private:
  const std::string         name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_MkdirsSync_h
