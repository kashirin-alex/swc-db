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


class MkdirsSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<MkdirsSync> Ptr;

  SWC_CAN_INLINE
  MkdirsSync(FS::Statistics& stats,
             uint32_t timeout, const std::string& a_name)
            : Base(
                stats, FS::Statistics::MKDIRS_SYNC,
                Buffers::make(
                  Params::MkdirsReq(a_name),
                  0,
                  FUNCTION_MKDIRS, timeout
                )
              ),
              name(a_name) {
  }

  ~MkdirsSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_mkdirs(ev, name);
    BaseSync::acknowledge();
  }

  private:
  const std::string         name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_MkdirsSync_h
