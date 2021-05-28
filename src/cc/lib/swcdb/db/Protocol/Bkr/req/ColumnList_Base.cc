/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/ColumnList_Base.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


ColumnList_Base::ColumnList_Base(
                      const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnListReq& params,
                      const uint32_t timeout)
                      : client::ConnQueue::ReqBase(
                          false,
                          Buffers::make(params, 0, COLUMN_LIST, timeout)
                        ),
                        clients(clients) {
}

void ColumnList_Base::handle_no_conn() {
  if(clients->stopping()) {
    callback(Error::CLIENT_STOPPING, Mngr::Params::ColumnListRsp());
  } else if(!valid()) {
    callback(Error::CANCELLED, Mngr::Params::ColumnListRsp());
  } else if(_bkr_idx.turn_around(clients->brokers)) {
    request_again();
  } else {
    run();
  }
}

bool ColumnList_Base::run() {
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

void ColumnList_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Mngr::Params::ColumnListRsp rsp;
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      rsp.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  } else if(err == Error::REQUEST_TIMEOUT) {
    SWC_LOG_OUT(LOG_INFO, Error::print(SWC_LOG_OSTREAM, err); );
    request_again();
    return;
  }

  callback(err, rsp);
}


}}}}}
