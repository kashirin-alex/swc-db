/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_ReadAll_h
#define swcdb_fs_Broker_Protocol_req_ReadAll_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class ReadAll final : public Base {
  public:
  typedef std::shared_ptr<ReadAll> Ptr;

  SWC_CAN_INLINE
  ReadAll(FS::Statistics& stats,
          uint32_t timeout, const std::string& a_name,
          FS::Callback::ReadAllCb_t&& a_cb)
          : Base(
              stats, FS::Statistics::READ_ALL_ASYNC,
              Buffers::make(
                Params::ReadAllReq(a_name),
                0,
                FUNCTION_READ_ALL, timeout
              )
            ),
            name(a_name), cb(std::move(a_cb)) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_read_all(ev, name);
    StaticBuffer::Ptr buf(error ? nullptr : new StaticBuffer(ev->data_ext));
    cb(error, name, buf);
  }

  private:
  const std::string                 name;
  const FS::Callback::ReadAllCb_t   cb;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_ReadAll_h
