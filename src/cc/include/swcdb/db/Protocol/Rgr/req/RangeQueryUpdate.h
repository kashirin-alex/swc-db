/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_RangeQueryUpdate_h
#define swcdb_db_protocol_rgr_req_RangeQueryUpdate_h


#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename DataT>
class RangeQueryUpdate final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeQueryUpdate> Ptr;
  DataT                                     data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RangeQueryUpdateReq& params,
        StaticBuffer& snd_buf,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new RangeQueryUpdate(
      Buffers::make(params, snd_buf, 0, RANGE_QUERY_UPDATE, timeout),
      args...
    ));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RangeQueryUpdateReq& params,
        const DynamicBuffer& buffer,
        const uint32_t timeout,
        DataArgsT&&... args) {
    StaticBuffer snd_buf(buffer.base, buffer.fill(), false);
    return make(params, snd_buf, timeout, args...);
  }

  SWC_CAN_INLINE
  static void request(const Ptr& req, const EndPoints& endpoints) {
    req->data.get_clients()->get_rgr_queue(endpoints)->put(req);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RangeQueryUpdateReq& params,
        StaticBuffer& snd_buf,
        const uint32_t timeout,
        const EndPoints& endpoints,
        DataArgsT&&... args) {
    request(make(params, snd_buf, timeout, args...), endpoints);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RangeQueryUpdateReq& params,
        const DynamicBuffer& buffer,
        const uint32_t timeout,
        const EndPoints& endpoints,
        DataArgsT&&... args) {
    request(make(params, buffer, timeout, args...), endpoints);
  }

  virtual ~RangeQueryUpdate() { }

  bool valid() override {
    return data.valid();
  }

  void handle_no_conn() override {
    data.callback(
      req(),
      Params::RangeQueryUpdateRsp(Error::COMM_NOT_CONNECTED)
    );
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    ev->type == Event::Type::DISCONNECT
      ? handle_no_conn()
      : data.callback(
          req(),
          Params::RangeQueryUpdateRsp(ev->error, ev->data.base, ev->data.size)
        );
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  RangeQueryUpdate(const Buffers::Ptr& cbp, DataArgsT&&... args)
                  : client::ConnQueue::ReqBase(false, cbp),
                    data(args...) {
  }

};



/** Functional_RangeQueryUpdate - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Rgr::Req::Functional_RangeQueryUpdate;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp&) {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Rgr::Req::RangeQueryUpdate<data_t>::request(
    params, buffer, timeout, endpoints, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    const Params::RangeQueryUpdateRsp&
  )>
> Functional_RangeQueryUpdate;




}}}}}


#endif // swcdb_db_protocol_rgr_req_RangeQueryUpdate_h
