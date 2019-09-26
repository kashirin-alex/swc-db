
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_ColumnMng_h
#define swc_lib_db_protocol_req_ColumnMng_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "MngrMngrActive.h"
#include "../params/ColumnMng.h"


namespace SWC {
namespace Protocol {
namespace Req {
namespace Column {

  
class Mng: public ConnQueue::ReqBase {
  public:

  using Func = Params::ColumnMng::Function;
  typedef std::function<void(ConnQueue::ReqBase::Ptr, int)> Cb_t;


  static void create(DB::SchemaPtr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::CREATE, schema, cb, timeout);
  }

  static void modify(DB::SchemaPtr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::MODIFY, schema, cb, timeout);
  }

  static void remove(DB::SchemaPtr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::DELETE, schema, cb, timeout);
  }

  static void request(Func func, DB::SchemaPtr schema, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<Mng>(
      Params::ColumnMng(func, schema),
      cb,
      timeout
    )->run();
  }


  Mng(const Params::ColumnMng& params, const Cb_t cb, const uint32_t timeout)
      : ConnQueue::ReqBase(false), cb(cb) {
    CommHeader header(Mngr::COLUMN_MNG, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~Mng(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      Env::Clients::get()->mngrs_groups->select(1, endpoints); // columns-get (can be any mngr)
      if(endpoints.empty()){
        std::make_shared<MngrMngrActive>(1, shared_from_this())->run();
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

    cb(req(), ev->error != Error::OK? ev->error: response_code(ev));
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

#endif // swc_lib_db_protocol_req_ColumnMng_h
