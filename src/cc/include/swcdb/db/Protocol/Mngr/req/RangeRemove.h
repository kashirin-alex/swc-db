/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RangeRemove_h
#define swcdb_db_protocol_req_RangeRemove_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/params/RangeRemove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename DataT>
class RangeRemove final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeRemove> Ptr;
  DataT                                data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RangeRemoveReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new RangeRemove(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RangeRemoveReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  virtual ~RangeRemove() noexcept { }

  bool valid() override {
    return data.valid();
  }

  void handle_no_conn() override {
    if(data.get_clients()->stopping()) {
      data.callback(
        req(), Params::RangeRemoveRsp(Error::CLIENT_STOPPING));
    } else if(!data.valid()) {
      data.callback(
        req(), Params::RangeRemoveRsp(Error::CANCELLED));
    } else {
      data.get_clients()->remove_mngr(endpoints);
      endpoints.clear();
      run();
    }
  }

  bool run() override {
    return data.get_clients()->managers.put(
      data.get_clients(), data.get_cid(), endpoints, req());
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    data.callback(
      req(),
      Params::RangeRemoveRsp(ev->error, ev->data.base, ev->data.size)
    );
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  RangeRemove(
        const Params::RangeRemoveReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : client::ConnQueue::ReqBase(
          Buffers::make(params, 0, RANGE_REMOVE, timeout)
        ),
        data(args...),
        endpoints() {
  }

  private:
  EndPoints                 endpoints;

};



}}}}}

#endif // swcdb_db_protocol_req_RangeRemove_h
