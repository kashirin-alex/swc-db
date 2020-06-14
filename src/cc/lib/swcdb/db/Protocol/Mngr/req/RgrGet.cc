
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

 
SWC_SHOULD_INLINE
void RgrGet::request(cid_t cid, rid_t rid, bool next_range,
                     const RgrGet::Cb_t cb, const uint32_t timeout) {
  request(Params::RgrGetReq(cid, rid, next_range), cb, timeout);
}

SWC_SHOULD_INLINE
void RgrGet::request(const Params::RgrGetReq params,
                     const RgrGet::Cb_t cb, const uint32_t timeout) {
  std::make_shared<RgrGet>(params, cb, timeout)->run();
}

SWC_SHOULD_INLINE
RgrGet::Ptr RgrGet::make(const Params::RgrGetReq params,
                         const RgrGet::Cb_t cb, const uint32_t timeout) {
  return std::make_shared<RgrGet>(params, cb, timeout);
}

RgrGet::RgrGet(const Params::RgrGetReq& params, const RgrGet::Cb_t cb, 
               const uint32_t timeout) 
              : client::ConnQueue::ReqBase(false), cb(cb), cid(params.cid) {
  cbp = CommBuf::make(params);
  cbp->header.set(RGR_GET, timeout);
}

RgrGet::~RgrGet(){}

void RgrGet::handle_no_conn() {
  clear_endpoints();
  run();
}

bool RgrGet::run(uint32_t timeout) {
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

void RgrGet::handle(ConnHandlerPtr conn, Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT){
    handle_no_conn();
    return;
  }

  Params::RgrGetRsp rsp_params;
  if(ev->type == Event::Type::ERROR){
    rsp_params.err = ev->error;
    cb(req(), rsp_params);
    return;
  }

  try{
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    rsp_params.decode(&ptr, &remain);
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }
  cb(req(), rsp_params);
}

void RgrGet::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}
