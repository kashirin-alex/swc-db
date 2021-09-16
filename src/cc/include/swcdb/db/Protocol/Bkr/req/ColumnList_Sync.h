/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnList_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnList_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnList.h"
#include "swcdb/core/StateSynchronization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnList_Sync {
  public:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static std::shared_ptr<ColumnList<ColumnList_Sync>>
  make(const Mngr::Params::ColumnListReq& params,
       const uint32_t timeout,
       DataArgsT&&... args) {
    return ColumnList<ColumnList_Sync>::make(params, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(const Mngr::Params::ColumnListReq& params,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    auto req = make(params, timeout, args...);
    req->run();
    req->data.await.wait();
  }


  Core::StateSynchronization        await;

  SWC_CAN_INLINE
  ColumnList_Sync(const SWC::client::Clients::Ptr& a_clients,
                 int& a_err, DB::SchemasVec& a_schemas) noexcept
                 : clients(a_clients), err(a_err), schemas(a_schemas) {
  }

  ~ColumnList_Sync() { }

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
                const Mngr::Params::ColumnListRsp& rsp) {
    if(!(err = error)) {
      if(rsp.expected) {
        Core::MutexSptd::scope lock(m_mutex);
        if(schemas.empty()) {
          schemas = std::move(rsp.schemas);
        } else {
          schemas.reserve(rsp.expected);
          schemas.insert(
            schemas.cend(), rsp.schemas.cbegin(), rsp.schemas.cend());
        }
        if(schemas.size() != rsp.expected)
          return;
      } else {
        schemas = std::move(rsp.schemas);
      }
    }
    await.acknowledge();
  }

  private:
  SWC::client::Clients::Ptr     clients;
  int&                          err;
  Core::MutexSptd               m_mutex;
  DB::SchemasVec&               schemas;

};



}}}}}



#endif // swcdb_db_protocol_bkr_req_ColumnList_Sync_h
