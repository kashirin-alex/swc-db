/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnMng_h
#define swcdb_db_protocol_bkr_req_ColumnMng_h


#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Base.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


template<typename DataT>
class ColumnMng final : public ColumnMng_Base {
  public:

  typedef std::shared_ptr<ColumnMng>  Ptr;
  DataT                               data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Mngr::Params::ColumnMng& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new ColumnMng(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Mngr::Params::ColumnMng& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(Mngr::Params::ColumnMng::Function func,
                      const DB::Schema::Ptr& schema,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    request(Mngr::Params::ColumnMng(func, schema), timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void create(const DB::Schema::Ptr& schema,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Mngr::Params::ColumnMng::Function::CREATE, schema, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void modify(const DB::Schema::Ptr& schema,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Mngr::Params::ColumnMng::Function::MODIFY, schema, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void remove(const DB::Schema::Ptr& schema,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Mngr::Params::ColumnMng::Function::DELETE, schema, timeout, args...);
  }

  virtual ~ColumnMng() { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    data.callback(req(), ev->response_code());
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  ColumnMng(
        const Mngr::Params::ColumnMng& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : ColumnMng_Base(params, timeout),
        data(args...) {
  }

  SWC::client::Clients::Ptr& get_clients() noexcept override {
    return data.get_clients();
  }

  bool valid() override {
    return data.valid();
  }

  void callback(int err) override {
    data.callback(req(), err);
  }

};



/** Functional_ColumnMng - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Bkr::Req::Functional_ColumnMng;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               int err) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Bkr::Req::ColumnMng<data_t>::request(
    params/(func, schema), timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    int err
  )>
> Functional_ColumnMng;



}}}}}



#endif // swcdb_db_protocol_bkr_req_ColumnMng_h
