/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Length_h
#define swcdb_fs_Broker_Protocol_req_Length_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Length final : public Base {
  public:
  typedef std::shared_ptr<Length> Ptr;

  SWC_CAN_INLINE
  Length(FS::Statistics& stats,
         uint32_t timeout, const std::string& a_name,
         FS::Callback::LengthCb_t&& a_cb)
        : Base(
            stats, FS::Statistics::LENGTH_ASYNC,
            Buffers::make(
              Params::LengthReq(a_name),
              0,
              FUNCTION_LENGTH, timeout
            )
          ),
          name(a_name), cb(std::move(a_cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    size_t length = 0;
    Base::handle_length(ev, name, length);
    cb(error, length);
  }

  private:
  const std::string               name;
  const FS::Callback::LengthCb_t  cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_Length_h
