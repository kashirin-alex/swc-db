/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Readdir_h
#define swcdb_fs_Broker_Protocol_req_Readdir_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Readdir final : public Base {
  public:
  typedef std::shared_ptr<Readdir> Ptr;

  SWC_CAN_INLINE
  Readdir(FS::Statistics& stats,
          uint32_t timeout, const std::string& a_name,
          FS::Callback::ReaddirCb_t&& a_cb)
          : Base(
              stats, FS::Statistics::READDIR_ASYNC,
              Buffers::make(
                Params::ReaddirReq(a_name),
                0,
                FUNCTION_READDIR, timeout
              )
            ),
            name(a_name), cb(std::move(a_cb)) {
  }

  ~Readdir() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    FS::DirentList listing;
    Base::handle_readdir(ev, name, listing);
    cb(error, std::move(listing));
  }

  private:
  const std::string                 name;
  const FS::Callback::ReaddirCb_t   cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Readdir_h
