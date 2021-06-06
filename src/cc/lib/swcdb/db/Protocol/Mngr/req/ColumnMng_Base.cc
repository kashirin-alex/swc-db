/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Base.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



ColumnMng_Base::ColumnMng_Base(const SWC::client::Clients::Ptr& clients,
                               const Params::ColumnMng& params,
                               const uint32_t timeout)
                    : client::ConnQueue::ReqBase(
                        Buffers::make(params, 0, COLUMN_MNG, timeout)
                      ),
                      clients(clients) {
}

void ColumnMng_Base::handle_no_conn() {
  if(clients->stopping()) {
    callback(Error::CLIENT_STOPPING);
  } else if(!valid()) {
    callback(Error::CANCELLED);
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnMng_Base::run() {
  if(endpoints.empty()) {
    clients->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping()) {
        callback(Error::CLIENT_STOPPING);
      } else if(!valid()) {
        callback(Error::CANCELLED);
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

void ColumnMng_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  callback(ev->response_code());
}

void ColumnMng_Base::clear_endpoints() {
  clients->remove_mngr(endpoints);
  endpoints.clear();
}


}}}}}
