/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_RenameSync_h
#define swcdb_fs_Broker_Protocol_req_RenameSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rename.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class RenameSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<RenameSync> Ptr;

  SWC_CAN_INLINE
  RenameSync(FS::Statistics& stats,
             uint32_t timeout,
             const std::string& a_from, const std::string& a_to)
            : Base(
                stats, FS::Statistics::RENAME_SYNC,
                Buffers::make(
                  Params::RenameReq(a_from, a_to),
                  0,
                  FUNCTION_RENAME, timeout
                )
              ),
              from(a_from), to(a_to) {
  }

  ~RenameSync() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_rename(ev, from, to);
    BaseSync::acknowledge();
  }

  private:
  const std::string from;
  const std::string to;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_RenameSync_h
