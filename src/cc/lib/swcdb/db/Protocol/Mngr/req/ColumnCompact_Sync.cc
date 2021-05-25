/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



SWC_SHOULD_INLINE
void ColumnCompact_Sync::request(const SWC::client::Clients::Ptr& clients,
                                 cid_t cid, int& err,
                                 const uint32_t timeout) {
  request(clients, Params::ColumnCompactReq(cid), err, timeout);
}

SWC_SHOULD_INLINE
void ColumnCompact_Sync::request(const SWC::client::Clients::Ptr& clients,
                                 const Params::ColumnCompactReq& params,
                                 int& err, const uint32_t timeout) {
  auto req = std::make_shared<ColumnCompact_Sync>(
    clients, params, err, timeout);
  auto res = req->await.get_future();
  req->run();
  res.get();
}


ColumnCompact_Sync::ColumnCompact_Sync(
                            const SWC::client::Clients::Ptr& clients,
                            const Params::ColumnCompactReq& params,
                            int& err, const uint32_t timeout)
                            : ColumnCompact_Base(clients, params, timeout),
                              err(err) {
}

void ColumnCompact_Sync::callback(const Params::ColumnCompactRsp& rsp) {
  err = rsp.err;
  await.set_value();
}



}}}}}
