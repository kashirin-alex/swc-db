
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/Report.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

 
Report::Report(Params::Report::Function func, const uint32_t timeout)
              : client::ConnQueue::ReqBase(false) {
  cbp = CommBuf::make(1);
  cbp->append_i8((uint8_t)func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const Serializable& params, 
               Params::Report::Function func, 
               const uint32_t timeout) 
              : client::ConnQueue::ReqBase(false) {
  cbp = CommBuf::make(params, 1);
  cbp->append_i8((uint8_t)func);
  cbp->header.set(REPORT, timeout);
}

Report::~Report() { }

void Report::handle_no_conn() {
  clear_endpoints();
  run();
}

void Report::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}



SWC_SHOULD_INLINE
void ColumnStatus::request(cid_t cid, const ColumnStatus::Cb_t& cb, 
                           const uint32_t timeout) {
  request(Params::Report::ReqColumnStatus(cid), cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnStatus::request(const Params::Report::ReqColumnStatus& params,
                           const ColumnStatus::Cb_t& cb, 
                           const uint32_t timeout) {
  std::make_shared<ColumnStatus>(params, cb, timeout)->run();
}

SWC_SHOULD_INLINE
ColumnStatus::Ptr 
ColumnStatus::make(const Params::Report::ReqColumnStatus& params,
                   const ColumnStatus::Cb_t& cb, const uint32_t timeout) {
  return std::make_shared<ColumnStatus>(params, cb, timeout);
}

ColumnStatus::ColumnStatus(const Params::Report::ReqColumnStatus& params, 
                           const Cb_t& cb, 
                           const uint32_t timeout) 
                          : Report(
                              params,
                              Params::Report::Function::COLUMN_STATUS,
                              timeout
                            ), 
                            cb(cb), cid(params.cid) {
}

ColumnStatus::~ColumnStatus() { }

bool ColumnStatus::run() {
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

void ColumnStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT){
    handle_no_conn();
    return;
  }
  
  Params::Report::RspColumnStatus rsp_params;
  int err = ev->type == Event::Type::ERROR ? ev->error : Error::OK;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}



SWC_SHOULD_INLINE
void RangersStatus::request(cid_t cid, const RangersStatus::Cb_t& cb, 
                            const uint32_t timeout) {
  std::make_shared<RangersStatus>(cid, cb, timeout)->run();
}

SWC_SHOULD_INLINE
RangersStatus::Ptr 
RangersStatus::make(cid_t cid, const RangersStatus::Cb_t& cb, 
                    const uint32_t timeout) {
  return std::make_shared<RangersStatus>(cid, cb, timeout);
}

RangersStatus::RangersStatus(cid_t cid, const Cb_t& cb, const uint32_t timeout) 
                            : Report(
                                Params::Report::Function::RANGERS_STATUS,
                                timeout
                              ), cb(cb), cid(cid) {
}

RangersStatus::~RangersStatus() { }

bool RangersStatus::run() {
  if(endpoints.empty()) {
    bool no_cid = cid == DB::Schema::NO_CID;
    if(no_cid)
      Env::Clients::get()->mngrs_groups->select(
        Types::MngrRole::RANGERS, endpoints);
    else
      Env::Clients::get()->mngrs_groups->select(cid, endpoints); 

    if(endpoints.empty()) {
      if(no_cid)
        MngrActive::make(Types::MngrRole::RANGERS, shared_from_this())->run();
      else
        MngrActive::make(cid, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void RangersStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT){
    handle_no_conn();
    return;
  }
  
  Params::Report::RspRangersStatus rsp_params;
  int err = ev->type == Event::Type::ERROR ? ev->error : Error::OK;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}





}}}}
