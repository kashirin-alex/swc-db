/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnMng_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnMng_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnMng.h"
#include "swcdb/core/StateSynchronization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnMng_Sync {
  public:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static std::shared_ptr<ColumnMng<ColumnMng_Sync>>
  make(const Mngr::Params::ColumnMng& params,
       const uint32_t timeout,
       DataArgsT&&... args) {
    return ColumnMng<ColumnMng_Sync>::make(params, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(const Mngr::Params::ColumnMng& params,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    auto req = make(params, timeout, args...);
    req->run();
    req->data.await.wait();
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

  Core::StateSynchronization        await;

  SWC_CAN_INLINE
  ColumnMng_Sync(const SWC::client::Clients::Ptr& a_clients,
                 int& a_err) noexcept
                : clients(a_clients), err(a_err) {
  }

  ~ColumnMng_Sync() noexcept { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return clients;
  }

  SWC_CAN_INLINE
  bool valid() {
    return true;
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&, int error) {
    err = error;
    await.acknowledge();
  }

  private:
  SWC::client::Clients::Ptr clients;
  int&                       err;

};



}}}}}



#endif // swcdb_db_protocol_bkr_req_ColumnMng_Sync_h
