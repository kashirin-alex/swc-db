/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_RangeQuerySelect_h
#define swcdb_db_protocol_rgr_req_RangeQuerySelect_h


#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename DataT>
class RangeQuerySelect final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeQuerySelect> Ptr;
  DataT                                     data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RangeQuerySelectReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new RangeQuerySelect(params, timeout, args...));
  }

  SWC_CAN_INLINE
  static void request(const Ptr& req, const EndPoints& endpoints) {
    req->data.get_clients()->get_rgr_queue(endpoints)->put(req);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RangeQuerySelectReq& params,
        const uint32_t timeout,
        const EndPoints& endpoints,
        DataArgsT&&... args) {
    request(make(params, timeout, args...), endpoints);
  }

  virtual ~RangeQuerySelect() { }

  bool valid() override {
    return data.valid();
  }

  void handle_no_conn() override {
    Params::RangeQuerySelectRsp rsp(Error::COMM_NOT_CONNECTED);
    data.callback(req(), rsp);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    if(ev->type == Event::Type::DISCONNECT)
      return handle_no_conn();

    Params::RangeQuerySelectRsp rsp(
      ev->error, ev->data.base, ev->data.size, ev->data_ext);
    data.callback(req(), rsp);
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  RangeQuerySelect(
        const Params::RangeQuerySelectReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
        : client::ConnQueue::ReqBase(
            false,
            Buffers::make(params, 0, RANGE_QUERY_SELECT, timeout)
          ),
          data(args...) {
  }

};



/** Functional_RangeQuerySelect - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Rgr::Req::Functional_RangeQuerySelect;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp&) {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Rgr::Req::RangeQuerySelect<data_t>::request(
    params, timeout, endpoints, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    const Params::RangeQuerySelectRsp&
  )>
> Functional_RangeQuerySelect;




}}}}}


#endif // swcdb_db_protocol_rgr_req_RangeQuerySelect_h
