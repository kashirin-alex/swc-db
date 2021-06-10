/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RgrGet_h
#define swcdb_db_protocol_req_RgrGet_h


#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename DataT>
class RgrGet final : public RgrGet_Base {
  public:

  typedef std::shared_ptr<RgrGet> Ptr;
  DataT                           data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::RgrGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new RgrGet(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::RgrGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        cid_t cid, rid_t rid, bool next_range,
        const uint32_t timeout,
        DataArgsT&&... args) {
    request(Params::RgrGetReq(cid, rid, next_range), timeout, args...);
  }

  virtual ~RgrGet() { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Params::RgrGetRsp rsp(ev->error, ev->data.base, ev->data.size);
    data.callback(req(), rsp);
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  RgrGet(
        const Params::RgrGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : RgrGet_Base(params, timeout),
        data(args...) {
  }

  SWC::client::Clients::Ptr& get_clients() noexcept override {
    return data.get_clients();
  }

  bool valid() override {
    return data.valid();
  }

  void callback(Params::RgrGetRsp& rsp) override {
    data.callback(req(), rsp);
  }

};



/** Functional_RgrGet - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Mngr::Req::Functional_RgrGet;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               Comm::Protocol::Mngr::Params::RgrGetRsp&) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Mngr::Req::RgrGet<data_t>::request(
    params, 10000, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    Params::RgrGetRsp&
  )>
> Functional_RgrGet;


}}}}}


#endif // swcdb_db_protocol_req_RgrGet_h
