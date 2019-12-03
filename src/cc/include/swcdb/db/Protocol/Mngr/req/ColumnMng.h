
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_mngr_req_ColumnMng_h
#define swc_lib_db_protocol_mngr_req_ColumnMng_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnMng: public Common::Req::ConnQueue::ReqBase {
  public:

  using Func = Params::ColumnMng::Function;
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, int)> Cb_t;


  static void create(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::CREATE, schema, cb, timeout);
  }

  static void modify(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::MODIFY, schema, cb, timeout);
  }

  static void remove(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000){
    request(Func::DELETE, schema, cb, timeout);
  }

  static void request(Func func, DB::Schema::Ptr schema, const Cb_t cb, 
                      const uint32_t timeout = 10000){
    std::make_shared<ColumnMng>(
      Params::ColumnMng(func, schema),
      cb,
      timeout
    )->run();
  }


  ColumnMng(const Params::ColumnMng& params, const Cb_t cb, const uint32_t timeout)
            : Common::Req::ConnQueue::ReqBase(false), cb(cb) {
    cbp = CommBuf::make(params);    
    cbp->header.set(COLUMN_MNG, timeout);
  }

  virtual ~ColumnMng(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      Env::Clients::get()->mngrs_groups->select(1, endpoints); // columns-get (can be any mngr)
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

    cb(req(), ev->error != Error::OK? ev->error: ev->response_code());
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

#endif // swc_lib_db_protocol_mngr_req_ColumnMng_h
