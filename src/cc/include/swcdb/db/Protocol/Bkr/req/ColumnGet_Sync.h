/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnGet_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnGet_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnGet_Sync {
  public:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static std::shared_ptr<ColumnGet<ColumnGet_Sync>> 
  make(const Mngr::Params::ColumnGetReq& params,
       const uint32_t timeout,
       DataArgsT&&... args) {
    return ColumnGet<ColumnGet_Sync>::make(params, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(const Mngr::Params::ColumnGetReq& params,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    auto req = make(params, timeout, args...);
    auto res = req->data.await.get_future();
    req->run();
    res.get();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void schema(const std::string& name,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Mngr::Params::ColumnGetReq(
        Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME, name),
      timeout,
      args...
    );
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void schema(cid_t cid,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Mngr::Params::ColumnGetReq(
        Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_ID, cid),
      timeout,
      args...
    );
  }

  std::promise<void>        await;

  SWC_CAN_INLINE
  ColumnGet_Sync(const SWC::client::Clients::Ptr& clients,
                 int& err, DB::Schema::Ptr& _schema) noexcept
                 : clients(clients), err(err), _schema(_schema) {
  }

  // ~ColumnGet_Sync() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return clients;
  }

  SWC_CAN_INLINE
  bool valid() {
    return true;
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&,
                int error,
                const Mngr::Params::ColumnGetRsp& rsp) {
    err = error;
    _schema = std::move(rsp.schema);
    await.set_value();
  }

  private:
  SWC::client::Clients::Ptr clients;
  int&                      err;
  DB::Schema::Ptr&          _schema;

};



}}}}}



#endif // swcdb_db_protocol_bkr_req_ColumnGet_Sync_h
