/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/Scanner_CellsSelect.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


Scanner_CellsSelect::Scanner_CellsSelect(
        const SWC::client::Query::Select::BrokerScanner::Ptr& scanner,
        const Params::CellsSelectReqRef& params)
        : client::ConnQueue::ReqBase(
            false,
            Buffers::make(params, 0, CELLS_SELECT, scanner->selector->timeout)
          ),
          scanner(scanner),
          profile(scanner->selector->profile.bkr()) {
}

void Scanner_CellsSelect::handle_no_conn() {
  if(scanner->selector->valid() &&
     !_bkr_idx.turn_around(scanner->selector->clients->brokers)) {
    run();
    return;
  }
  Params::CellsSelectRsp rsp(Error::COMM_NOT_CONNECTED);
  profile.add(rsp.err);
  scanner->selected(req(), rsp);
}

bool Scanner_CellsSelect::run() {
  auto& clients = scanner->selector->clients;
  EndPoints endpoints;
  while(scanner->selector->valid() &&
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

void Scanner_CellsSelect::handle(ConnHandlerPtr,
                                 const Event::Ptr& ev) {
  Params::CellsSelectRsp rsp_params(ev->error, ev->data_ext);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }
  profile.add(rsp_params.err);
  scanner->selected(req(), rsp_params);
}


}}}}}
