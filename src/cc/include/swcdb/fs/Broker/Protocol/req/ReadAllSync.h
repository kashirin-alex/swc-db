/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReadAllSync_h
#define swcdb_fs_Broker_Protocol_req_ReadAllSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReadAllSync final : public BaseSync, public Base {
  public:
  typedef std::shared_ptr<ReadAllSync> Ptr;

  StaticBuffer* buffer;

  SWC_CAN_INLINE
  ReadAllSync(FS::Statistics& stats,
              uint32_t timeout, const std::string& a_name, StaticBuffer* dst)
              : Base(
                  stats, FS::Statistics::READ_ALL_SYNC,
                  Buffers::make(
                    Params::ReadAllReq(a_name),
                    0,
                    FUNCTION_READ_ALL, timeout
                  )
                ),
                buffer(dst), name(a_name) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_read_all(ev, name);
    if(!error)
      buffer->set(ev->data_ext);
    BaseSync::acknowledge();
  }

  private:
  const std::string                 name;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_ReadAllSync_h
