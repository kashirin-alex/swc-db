/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnList_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnList_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnList.h"


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
    auto res = req->data.await.get_future();
    req->run();
    res.get();
  }


  std::promise<void>        await;

  SWC_CAN_INLINE
  ColumnList_Sync(const SWC::client::Clients::Ptr& clients,
                 int& err, std::vector<DB::Schema::Ptr>& schemas) noexcept
                 : clients(clients), err(err), schemas(schemas) {
  }

  // ~ColumnList_Sync() { }

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
    err = error;
    schemas = std::move(rsp.schemas);
    await.set_value();
  }

  private:
  SWC::client::Clients::Ptr     clients;
  int&                          err;
  std::vector<DB::Schema::Ptr>& schemas;

};



}}}}}



#endif // swcdb_db_protocol_bkr_req_ColumnList_Sync_h
