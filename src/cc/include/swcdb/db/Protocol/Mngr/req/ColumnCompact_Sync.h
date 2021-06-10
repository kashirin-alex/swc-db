/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_ColumnCompact_Sync_h
#define swcdb_db_protocol_req_ColumnCompact_Sync_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnCompact_Sync {
  public:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static std::shared_ptr<ColumnCompact<ColumnCompact_Sync>> 
  make(const Params::ColumnCompactReq& params,
       const uint32_t timeout,
       DataArgsT&&... args) {
    return ColumnCompact<ColumnCompact_Sync>::make(params, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(const Params::ColumnCompactReq& params,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    auto req = make(params, timeout, args...);
    auto res = req->data.await.get_future();
    req->run();
    res.get();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(cid_t cid,
                      const uint32_t timeout,
                      DataArgsT&&... args) {
    request(Params::ColumnCompactReq(cid), timeout, args...);
  }

  std::promise<void>       await;

  SWC_CAN_INLINE
  ColumnCompact_Sync(const SWC::client::Clients::Ptr& clients,
                     int& err) noexcept
                    : clients(clients), err(err) {
  }

  // ~ColumnCompact_Sync() { }

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
                const Params::ColumnCompactRsp& rsp) {
    err = rsp.err;
    await.set_value();
  }

  private:
  SWC::client::Clients::Ptr clients;
  int&                       err;

};



}}}}}



#endif // swcdb_db_protocol_req_ColumnCompact_Sync_h
