/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_ColumnCompact_h
#define swcdb_db_protocol_req_ColumnCompact_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Base.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename DataT>
class ColumnCompact final : public ColumnCompact_Base {
  public:

  typedef std::shared_ptr<ColumnCompact>  Ptr;
  DataT                                   data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::ColumnCompactReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new ColumnCompact(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::ColumnCompactReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(cid_t cid,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    request(Params::ColumnCompactReq(cid), timeout, args...);
  }

  virtual ~ColumnCompact() noexcept { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    data.callback(
      req(),
      Params::ColumnCompactRsp(ev->error, ev->data.base, ev->data.size)
    );
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  ColumnCompact(
        const Params::ColumnCompactReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : ColumnCompact_Base(params, timeout),
        data(args...) {
  }

  SWC::client::Clients::Ptr& get_clients() noexcept override {
    return data.get_clients();
  }

  bool valid() override {
    return data.valid();
  }

  void callback(const Params::ColumnCompactRsp& rsp) override {
    data.callback(req(), rsp);
  }

};



/** Functional_ColumnCompact - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Mngr::Req::Functional_ColumnCompact;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               const Comm::Protocol::Mngr::Params::ColumnCompactRsp&) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Mngr::Req::ColumnCompact<data_t>::request(
    params/cid/name, timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    Params::ColumnCompactRsp&
  )>
> Functional_ColumnCompact;



}}}}}



#endif // swcdb_db_protocol_req_ColumnCompact_h
