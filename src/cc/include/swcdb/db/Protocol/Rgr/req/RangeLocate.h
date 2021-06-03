/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_protocol_rgr_req_RangeLocate_h
#define swcdb_db_protocol_rgr_req_RangeLocate_h


#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename DataT>
class RangeLocate final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeLocate> Ptr;
  DataT                                data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RangeLocateReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new RangeLocate(params, timeout, args...));
  }

  SWC_CAN_INLINE
  static void request(const Ptr& req, const EndPoints& endpoints) {
    req->data.get_clients()->get_rgr_queue(endpoints)->put(req);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RangeLocateReq& params,
        const EndPoints& endpoints,
        const uint32_t timeout,
        DataArgsT&&... args) {
    request(make(params, timeout, args...), endpoints);
  }

  virtual ~RangeLocate() { }

  bool valid() override {
    return data.valid();
  }

  void handle_no_conn() override {
    data.callback(
      req(),
      Params::RangeLocateRsp(Error::COMM_NOT_CONNECTED)
    );
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    ev->type == Event::Type::DISCONNECT
      ? handle_no_conn()
      : data.callback(
          req(),
          Params::RangeLocateRsp(ev->error, ev->data.base, ev->data.size)
        );
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  RangeLocate(
        const Params::RangeLocateReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : client::ConnQueue::ReqBase(
          false,
          Buffers::make(params, 0, RANGE_LOCATE, timeout)
        ),
        data(args...) {
  }

};



/** Functional_RangeLocate - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Rgr::Req::Functional_RangeLocate;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               const Comm::Protocol::Rgr::Params::RangeLocateRsp&) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Rgr::Req::RangeLocate<data_t>::request(
    params, endpoints, 10000, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    const Params::RangeLocateRsp&
  )>
> Functional_RangeLocate;



}}}}}


#endif // swcdb_db_protocol_rgr_req_RangeLocate_h
