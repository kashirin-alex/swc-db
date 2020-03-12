
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {



void ColumnCompact::request(int64_t cid, const ColumnCompact::Cb_t cb, 
                            const uint32_t timeout) {
  request(Params::ColumnCompactReq(cid), cb, timeout);
}

void ColumnCompact::request(const Params::ColumnCompactReq params,
                            const ColumnCompact::Cb_t cb, 
                            const uint32_t timeout) {
  std::make_shared<ColumnCompact>(params, cb, timeout)->run();
}


ColumnCompact::ColumnCompact(const Params::ColumnCompactReq& params, 
                             const ColumnCompact::Cb_t cb, 
                             const uint32_t timeout) 
                            : client::ConnQueue::ReqBase(false), 
                              cb(cb), cid(params.cid) {
  cbp = CommBuf::make(params);
  cbp->header.set(COLUMN_COMPACT, timeout);
}

ColumnCompact::~ColumnCompact() { }

void ColumnCompact::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnCompact::run(uint32_t timeout) {
  if(endpoints.empty()){
    Env::Clients::get()->mngrs_groups->select(cid, endpoints); 
    if(endpoints.empty()){
      std::make_shared<MngrActive>(cid, shared_from_this())->run();
      return false;
    }
  } 
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnCompact::handle(ConnHandlerPtr conn, Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT){
    handle_no_conn();
    return;
  }

  Params::ColumnCompactRsp rsp_params;
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

void ColumnCompact::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}



}}}}
