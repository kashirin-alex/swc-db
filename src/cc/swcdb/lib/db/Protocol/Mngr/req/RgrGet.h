
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_RgrGet_h
#define swc_lib_db_protocol_req_RgrGet_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "MngrActive.h"
#include "../params/RgrGet.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class RgrGet: public Common::Req::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, 
                              Params::RgrGetRsp)> Cb_t;

  static void request(int64_t cid, int64_t rid, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    request(Params::RgrGetReq(cid, rid), cb, timeout);
  }

  static void request(int64_t cid, const DB::Specs::Interval& interval, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    request(Params::RgrGetReq(cid, interval), cb, timeout);
  }

  static inline void request(const Params::RgrGetReq params,
                             const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<RgrGet>(params, cb, timeout)->run();
  }


  RgrGet(const Params::RgrGetReq& params, const Cb_t cb, 
            const uint32_t timeout) 
            : Common::Req::ConnQueue::ReqBase(false), cb(cb), cid(params.cid) {
    cbp = CommBuf::make(params);
    cbp->header.set(RGR_GET, timeout);
  }

  virtual ~RgrGet(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
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

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

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

  private:
  
  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Cb_t  cb;
  EndPoints   endpoints;
  int64_t     cid;
};


}}}}

#endif // swc_lib_db_protocol_req_RgrGet_h
