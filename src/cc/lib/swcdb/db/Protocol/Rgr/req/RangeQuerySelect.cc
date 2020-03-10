
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */



#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
void 
RangeQuerySelect::request(const Params::RangeQuerySelectReq& params,
                          const EndPoints& endpoints, 
                          const RangeQuerySelect::Cb_t cb,
                          const uint32_t timeout) {
  std::make_shared<RangeQuerySelect>(params, endpoints, cb, timeout)->run();
}

RangeQuerySelect::RangeQuerySelect(const Params::RangeQuerySelectReq& params,
                                   const EndPoints& endpoints, 
                                   const RangeQuerySelect::Cb_t cb, 
                                   const uint32_t timeout) 
                                  : client::ConnQueue::ReqBase(false), 
                                    endpoints(endpoints), cb(cb) {
  cbp = CommBuf::make(params);
  cbp->header.set(RANGE_QUERY_SELECT, timeout);
}

RangeQuerySelect::~RangeQuerySelect() { }

void RangeQuerySelect::handle_no_conn() {
  cb(req(), Params::RangeQuerySelectRsp(Error::COMM_NOT_CONNECTED));
}

bool RangeQuerySelect::run(uint32_t timeout) {
  Env::Clients::get()->rgr->get(endpoints)->put(req());
  return true;
}

void RangeQuerySelect::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
  
  if(ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  Params::RangeQuerySelectRsp rsp_params(ev->data_ext);
  if(ev->type == Event::Type::ERROR) {
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

}}}}