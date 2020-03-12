
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_lib_db_protocol_req_RangeUnloaded_h
#define swc_lib_db_protocol_req_RangeUnloaded_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RangeUnloaded.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class RangeUnloaded: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(client::ConnQueue::ReqBase::Ptr, 
                             const Params::RangeUnloadedRsp&)> Cb_t;
 
  static void request(int64_t cid, int64_t rid, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    request(Params::RangeUnloadedReq(cid, rid), cb, timeout);
  }

  static inline void request(const Params::RangeUnloadedReq params,
                             const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<RangeUnloaded>(params, cb, timeout)->run();
  }


  RangeUnloaded(const Params::RangeUnloadedReq& params, const Cb_t cb, 
              const uint32_t timeout) 
              : client::ConnQueue::ReqBase(false), 
                cb(cb), cid(params.cid) {
    cbp = CommBuf::make(params);
    cbp->header.set(RANGE_UNLOADED, timeout);
  }

  virtual ~RangeUnloaded(){}

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

    Params::RangeUnloadedRsp rsp_params;
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

  const Cb_t      cb;
  const int64_t   cid;
  EndPoints       endpoints;
};


}}}}

#endif // swc_lib_db_protocol_req_RangeUnloaded_h
