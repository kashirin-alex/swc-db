
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */



#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


void ColumnMng::create(DB::Schema::Ptr schema, const ColumnMng::Cb_t cb, 
                      const uint32_t timeout) {
  request(Func::CREATE, schema, cb, timeout);
}

 void ColumnMng::modify(DB::Schema::Ptr schema, const ColumnMng::Cb_t cb, 
                        const uint32_t timeout) {
  request(Func::MODIFY, schema, cb, timeout);
}

 void ColumnMng::remove(DB::Schema::Ptr schema, const ColumnMng::Cb_t cb, 
                        const uint32_t timeout) {
  request(Func::DELETE, schema, cb, timeout);
}

 void ColumnMng::request(ColumnMng::Func func, DB::Schema::Ptr schema, 
                         const ColumnMng::Cb_t cb, const uint32_t timeout) {
  std::make_shared<ColumnMng>(Params::ColumnMng(func, schema), cb, timeout)
    ->run();
}


ColumnMng::ColumnMng(const Params::ColumnMng& params, const ColumnMng::Cb_t cb, 
                     const uint32_t timeout)
                    : client::ConnQueue::ReqBase(false), cb(cb) {
  cbp = CommBuf::make(params);    
  cbp->header.set(COLUMN_MNG, timeout);
}

ColumnMng::~ColumnMng() { }

void ColumnMng::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnMng::run(uint32_t timeout) {
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

void ColumnMng::handle(ConnHandlerPtr conn, Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT){
    handle_no_conn();
    return;
  }

  cb(req(), ev->error != Error::OK? ev->error: ev->response_code());
}

void ColumnMng::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}
