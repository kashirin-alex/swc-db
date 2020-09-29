
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnMng::create(const DB::Schema::Ptr& schema, 
                       const ColumnMng::Cb_t& cb, const uint32_t timeout) {
  request(Func::CREATE, schema, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::modify(const DB::Schema::Ptr& schema, 
                       const ColumnMng::Cb_t& cb, const uint32_t timeout) {
  request(Func::MODIFY, schema, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::remove(const DB::Schema::Ptr& schema, 
                       const ColumnMng::Cb_t& cb, const uint32_t timeout) {
  request(Func::DELETE, schema, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::request(ColumnMng::Func func, const DB::Schema::Ptr& schema,
                        const ColumnMng::Cb_t& cb, const uint32_t timeout) {
  std::make_shared<ColumnMng>(Params::ColumnMng(func, schema), cb, timeout)
    ->run();
}


ColumnMng::ColumnMng(const Params::ColumnMng& params, 
                     const ColumnMng::Cb_t& cb, const uint32_t timeout)
                    : Comm::client::ConnQueue::ReqBase(false), cb(cb) {
  cbp = Comm::CommBuf::make(params);    
  cbp->header.set(COLUMN_MNG, timeout);
}

ColumnMng::~ColumnMng() { }

void ColumnMng::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnMng::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(
      Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(Types::MngrRole::SCHEMAS, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnMng::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) {
  if(ev->type == Comm::Event::Type::DISCONNECT)
    return handle_no_conn();

  cb(req(), ev->response_code());
}

void ColumnMng::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}
