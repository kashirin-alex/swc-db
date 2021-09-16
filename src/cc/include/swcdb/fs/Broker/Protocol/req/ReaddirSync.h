/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReaddirSync_h
#define swcdb_fs_Broker_Protocol_req_ReaddirSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReaddirSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<ReaddirSync> Ptr;

  FS::DirentList& listing;

  SWC_CAN_INLINE
  ReaddirSync(FS::Statistics& stats,
              uint32_t timeout, const std::string& a_name,
              FS::DirentList& a_listing)
              : Base(
                  stats, FS::Statistics::READDIR_SYNC,
                  Buffers::make(
                    Params::ReaddirReq(a_name),
                    0,
                    FUNCTION_READDIR, timeout
                  )
                ),
                listing(a_listing), name(a_name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_readdir(ev, name, listing);
    BaseSync::acknowledge();
  }


  private:
  const std::string name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_ReaddirSync_h
