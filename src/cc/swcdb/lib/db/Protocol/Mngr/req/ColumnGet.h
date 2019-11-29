
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_req_ColumnGet_h
#define swc_db_protocol_mngr_req_ColumnGet_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "MngrActive.h"
#include "../params/ColumnGet.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnGet: public Common::Req::ConnQueue::ReqBase {
  public:
  
  using Flag = Params::ColumnGetReq::Flag;
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, 
                            int, Params::ColumnGetRsp)> Cb_t;


  static void schema(const std::string& name, const Cb_t cb, 
                     const uint32_t timeout = 10000) {
    request(Flag::SCHEMA_BY_NAME, name, cb, timeout);
  }
  
  static void schema(int64_t cid, const Cb_t cb, 
                     const uint32_t timeout = 10000) {
    request(Flag::SCHEMA_BY_ID, cid, cb, timeout);
  }

  static void cid(const std::string& name, const Cb_t cb, 
                  const uint32_t timeout = 10000) {
    request(Flag::ID_BY_NAME, name, cb, timeout);
  }

  static void request(Flag flag, const std::string& name, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<ColumnGet>(
      Params::ColumnGetReq(flag, name), cb, timeout
    )->run();
  }

  static void request(Flag flag, int64_t cid, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<ColumnGet>(
      Params::ColumnGetReq(flag, cid), cb, timeout
    )->run();
  }


  ColumnGet(const Params::ColumnGetReq& params, const Cb_t cb, 
            const uint32_t timeout) 
            : Common::Req::ConnQueue::ReqBase(false), cb(cb) {
    cbp = CommBuf::make(params);
    cbp->header.set(COLUMN_GET, timeout);
  }

  virtual ~ColumnGet(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      // columns-get (can be any mngr)
      Env::Clients::get()->mngrs_groups->select(1, endpoints); 
      if(endpoints.empty()){
        std::make_shared<MngrActive>(1, shared_from_this())->run();
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

    Params::ColumnGetRsp rsp_params;
    int err = ev->error != Error::OK? ev->error: response_code(ev);

    if(err == Error::OK){
      try{
        const uint8_t *ptr = ev->data.base+4;
        size_t remain = ev->data.size-4;
        rsp_params.decode(&ptr, &remain);
      } catch (Exception &e) {
        SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
        err = e.code();
      }
    }

    cb(req(), err, rsp_params);
  }

  private:
  
  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Cb_t  cb;
  EndPoints   endpoints;
};



}}}}

#endif // swc_db_protocol_mngr_req_ColumnGet_h
