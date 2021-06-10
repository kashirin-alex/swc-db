/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnGet_h
#define swcdb_db_protocol_mngr_req_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Base.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename DataT>
class ColumnGet final : public ColumnGet_Base {
  public:

  typedef std::shared_ptr<ColumnGet>  Ptr;
  DataT                               data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::ColumnGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new ColumnGet(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::ColumnGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(Params::ColumnGetReq::Flag flag,
                      const std::string& name,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    request(Params::ColumnGetReq(flag, name), timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(Params::ColumnGetReq::Flag flag,
                      cid_t cid,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    request(Params::ColumnGetReq(flag, cid), timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void schema(const std::string& name,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Params::ColumnGetReq::Flag::SCHEMA_BY_NAME, name, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void schema(cid_t cid,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Params::ColumnGetReq::Flag::SCHEMA_BY_ID, cid, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void cid(const std::string& name,
                  const uint32_t timeout,
                  DataArgsT&&... args) {
    request(
      Params::ColumnGetReq::Flag::ID_BY_NAME, name, timeout, args...);
  }

  virtual ~ColumnGet() { }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  ColumnGet(
        const Params::ColumnGetReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : ColumnGet_Base(params, timeout),
        data(args...) {
  }

  SWC::client::Clients::Ptr& get_clients() noexcept override {
    return data.get_clients();
  }

  bool valid() override {
    return data.valid();
  }

  void callback(int err, const Params::ColumnGetRsp& rsp) override {
    data.callback(req(), err, rsp);
  }

};



/** Functional_ColumnGet - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Mngr::Req::Functional_ColumnGet;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               int err,
               const Comm::Protocol::Mngr::Params::ColumnGetRsp&) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Mngr::Req::ColumnGet<data_t>::request(
    params/cid/name, timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    int err,
    Params::ColumnGetRsp&
  )>
> Functional_ColumnGet;


}}}}}


#endif // swcdb_db_protocol_mngr_req_ColumnGet_h
