
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_GetColumn_h
#define swc_lib_db_protocol_req_GetColumn_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "ActiveMngrRoute.h"
#include "../params/GetColumn.h"


namespace SWC {
namespace Protocol {
namespace Req {
namespace Column {

  
class Get: public ConnQueue::ReqBase {
  public:
  
  using Flag = Params::GetColumnReq::Flag;
  typedef std::function<
            void(ConnQueue::ReqBase::Ptr, int, Params::GetColumnRsp)> Cb_t;


  static void scheme(std::string& name, const Cb_t cb, 
                     const uint32_t timeout = 10000) {
    request(Flag::SCHEMA_BY_NAME, name, cb, timeout);
  }
  
  static void scheme(int64_t cid, const Cb_t cb, 
                     const uint32_t timeout = 10000) {
    request(Flag::SCHEMA_BY_ID, cid, cb, timeout);
  }

  static void cid(std::string& name, const Cb_t cb, 
                  const uint32_t timeout = 10000) {
    request(Flag::ID_BY_NAME, name, cb, timeout);
  }

  static void request(Flag flag, std::string& name, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<Get>(
      Protocol::Params::GetColumnReq(flag, name), cb, timeout
    )->run();
  }

  static void request(Flag flag, int64_t cid, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<Get>(
      Protocol::Params::GetColumnReq(flag, cid), cb, timeout
    )->run();
  }


  Get(const Protocol::Params::GetColumnReq params, const Cb_t cb, 
      const uint32_t timeout) : ConnQueue::ReqBase(false), cb(cb) {

    CommHeader header(Protocol::Command::CLIENT_REQ_GET_COLUMN, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~Get(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      // columns-get (can be any mngr)
      Env::Clients::get()->mngrs_groups->select(1, endpoints); 
      if(endpoints.empty()){
        std::make_shared<ActiveMngrRoute>(1, shared_from_this())->run();
        return false;
      }
    } 
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Protocol::Params::GetColumnRsp rsp_params;
    int err = ev->error != Error::OK? ev->error: Protocol::response_code(ev);

    if(err == Error::OK){
      try{
        const uint8_t *ptr = ev->payload+4;
        size_t remain = ev->payload_len-4;
        rsp_params.decode(&ptr, &remain);
      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
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


}
}}}

#endif // swc_lib_db_protocol_req_GetColumn_h
