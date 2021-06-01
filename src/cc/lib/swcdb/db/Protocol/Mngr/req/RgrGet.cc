/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void RgrGet::request(const SWC::client::Clients::Ptr& clients,
                     cid_t cid,
                     rid_t rid,
                     bool next_range,
                     RgrGet::Cb_t&& cb,
                     const uint32_t timeout) {
  request(
    clients, Params::RgrGetReq(cid, rid, next_range), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void RgrGet::request(const SWC::client::Clients::Ptr& clients,
                     const Params::RgrGetReq& params,
                     RgrGet::Cb_t&& cb,
                     const uint32_t timeout) {
  make(clients, params, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
RgrGet::Ptr RgrGet::make(const SWC::client::Clients::Ptr& clients,
                         const Params::RgrGetReq& params,
                         RgrGet::Cb_t&& cb,
                         const uint32_t timeout) {
  return Ptr(new RgrGet(clients, params, std::move(cb), timeout));
}

RgrGet::RgrGet(const SWC::client::Clients::Ptr& clients,
               const Params::RgrGetReq& params,
               RgrGet::Cb_t&& cb,
               const uint32_t timeout)
              : RgrGet_Base(params, timeout),
                clients(clients), cb(std::move(cb)) {
}

void RgrGet::callback(const Params::RgrGetRsp& rsp) {
  cb(req(), rsp);
}

}}}}}
