/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Mkdirs_h
#define swcdb_fs_Broker_Protocol_req_Mkdirs_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Mkdirs final : public Base {
  public:
  typedef std::shared_ptr<Mkdirs> Ptr;

  SWC_CAN_INLINE
  Mkdirs(FS::Statistics& stats,
         uint32_t timeout, const std::string& name,
         FS::Callback::MkdirsCb_t&& cb)
        : Base(
            stats, FS::Statistics::MKDIRS_ASYNC,
            Buffers::make(
              Params::MkdirsReq(name),
              0,
              FUNCTION_MKDIRS, timeout
            )
          ),
          name(name), cb(std::move(cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_mkdirs(ev, name);
    cb(error);
  }

  private:
  const std::string               name;
  const FS::Callback::MkdirsCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Mkdirs_h
