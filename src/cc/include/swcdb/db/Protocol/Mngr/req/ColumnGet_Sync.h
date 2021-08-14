/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnGet_Sync_h
#define swcdb_db_protocol_mngr_req_ColumnGet_Sync_h


#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/core/StateSynchronization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnGet_Sync {
  public:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static std::shared_ptr<ColumnGet<ColumnGet_Sync>>
  make(const Params::ColumnGetReq& params,
       const uint32_t timeout,
       DataArgsT&&... args) {
    return ColumnGet<ColumnGet_Sync>::make(params, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(const Params::ColumnGetReq& params,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    auto req = make(params, timeout, args...);
    req->run();
    req->data.await.wait();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void schema(const std::string& name,
                     const uint32_t timeout,
                     DataArgsT&&... args) {
    request(
      Params::ColumnGetReq(Params::ColumnGetReq::Flag::SCHEMA_BY_NAME, name),
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
      Params::ColumnGetReq(Params::ColumnGetReq::Flag::SCHEMA_BY_ID, cid),
      timeout,
      args...
    );
  }

  Core::StateSynchronization        await;

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
                const Params::ColumnGetRsp& rsp) {
    err = error;
    _schema = std::move(rsp.schema);
    await.acknowledge();
  }

  private:
  SWC::client::Clients::Ptr clients;
  int&                      err;
  DB::Schema::Ptr&          _schema;

};



}}}}}


#endif // swcdb_db_protocol_mngr_req_ColumnGet_Sync_h
