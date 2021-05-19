/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnMng::create(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::CREATE, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::modify(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::MODIFY, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::remove(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::DELETE, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::request(const SWC::client::Clients::Ptr& clients,
                        ColumnMng::Func func, const DB::Schema::Ptr& schema,
                        ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  std::make_shared<ColumnMng>(
    clients, Params::ColumnMng(func, schema), std::move(cb), timeout)->run();
}


ColumnMng::ColumnMng(const SWC::client::Clients::Ptr& clients,
                     const Params::ColumnMng& params,
                     ColumnMng::Cb_t&& cb, const uint32_t timeout)
                    : client::ConnQueue::ReqBase(
                        false,
                        Buffers::make(params, 0, COLUMN_MNG, timeout)
                      ),
                      clients(clients), cb(std::move(cb)) {
}

void ColumnMng::handle_no_conn() {
  if(clients->stopping()) {
    cb(req(), Error::CLIENT_STOPPING);
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnMng::run() {
  if(endpoints.empty()) {
    clients->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping()) {
        cb(req(), Error::CLIENT_STOPPING);
      } else {
        MngrActive::make(
          clients, DB::Types::MngrRole::SCHEMAS, shared_from_this())->run();
      }
      return false;
    }
  }
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ColumnMng::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  cb(req(), ev->response_code());
}

void ColumnMng::clear_endpoints() {
  clients->remove_mngr(endpoints);
  endpoints.clear();
}


}}}}}
