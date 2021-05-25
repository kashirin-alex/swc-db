/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



SWC_SHOULD_INLINE
void ColumnCompact::request(const SWC::client::Clients::Ptr& clients,
                            cid_t cid, ColumnCompact::Cb_t&& cb,
                            const uint32_t timeout) {
  request(
    clients, Params::ColumnCompactReq(cid), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnCompact::request(const SWC::client::Clients::Ptr& clients,
                            const Params::ColumnCompactReq& params,
                            ColumnCompact::Cb_t&& cb,
                            const uint32_t timeout) {
  std::make_shared<ColumnCompact>(
    clients, params, std::move(cb), timeout)->run();
}


ColumnCompact::ColumnCompact(const SWC::client::Clients::Ptr& clients,
                             const Params::ColumnCompactReq& params,
                             ColumnCompact::Cb_t&& cb,
                             const uint32_t timeout)
                            : ColumnCompact_Base(clients, params, timeout),
                              cb(std::move(cb)) {
}

void ColumnCompact::callback(const Params::ColumnCompactRsp& rsp) {
  cb(req(), rsp);
}


}}}}}
