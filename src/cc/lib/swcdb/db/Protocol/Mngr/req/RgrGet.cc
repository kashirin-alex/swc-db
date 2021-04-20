/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void RgrGet::request(cid_t cid,
                     rid_t rid,
                     bool next_range,
                     RgrGet::Cb_t&& cb,
                     const uint32_t timeout) {
  request(Params::RgrGetReq(cid, rid, next_range), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void RgrGet::request(const Params::RgrGetReq& params,
                     RgrGet::Cb_t&& cb,
                     const uint32_t timeout) {
  std::make_shared<RgrGet>(params, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
RgrGet::Ptr RgrGet::make(const Params::RgrGetReq& params,
                         RgrGet::Cb_t&& cb,
                         const uint32_t timeout) {
  return std::make_shared<RgrGet>(params, std::move(cb), timeout);
}

RgrGet::RgrGet(const Params::RgrGetReq& params,
               RgrGet::Cb_t&& cb,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  false,
                  Buffers::make(params, 0 ,RGR_GET, timeout)
                ),
                cb(std::move(cb)), cid(params.cid) {
}

void RgrGet::handle_no_conn() {
  clear_endpoints();
  run();
}

bool RgrGet::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(cid, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(cid, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void RgrGet::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::RgrGetRsp rsp_params(ev->error);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }

  cb(req(), rsp_params);
}

void RgrGet::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}}
