/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Base.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {



ColumnMng_Base::ColumnMng_Base(const SWC::client::Clients::Ptr& clients,
                               const Mngr::Params::ColumnMng& params,
                               const uint32_t timeout)
                    : client::ConnQueue::ReqBase(
                        false,
                        Buffers::make(params, 0, COLUMN_MNG, timeout)
                      ),
                      clients(clients) {
}

void ColumnMng_Base::handle_no_conn() {
  if(clients->stopping()) {
    callback(Error::CLIENT_STOPPING);
  } else if(!valid()) {
    callback(Error::CANCELLED);
  } else if(_bkr_idx.turn_around(clients->brokers)) {
    request_again();
  } else {
    run();
  }
}

bool ColumnMng_Base::run() {
  EndPoints endpoints;
  while(!clients->stopping() &&
        valid() &&
        (endpoints = clients->brokers.get_endpoints(_bkr_idx)).empty()) {
    SWC_LOG(LOG_ERROR,
      "Broker hosts cfg 'swc.bkr.host' is empty, waiting!");
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  if(endpoints.empty()) {
    handle_no_conn();
    return false;
  }
  clients->get_bkr_queue(endpoints)->put(req());
  return true;
}

void ColumnMng_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  callback(ev->response_code());
}


}}}}}
