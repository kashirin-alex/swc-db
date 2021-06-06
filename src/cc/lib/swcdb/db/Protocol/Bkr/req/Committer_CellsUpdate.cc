/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/Committer_CellsUpdate.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


void Committer_CellsUpdate::handle_no_conn() {
  if(committer->hdlr->valid() &&
     !_bkr_idx.turn_around(committer->hdlr->clients->brokers)) {
    run();
    return;
  }
  Params::CellsUpdateRsp rsp(Error::COMM_NOT_CONNECTED);
  profile.add(rsp.err);
  committer->committed(req(), rsp, buffer);
}

bool Committer_CellsUpdate::run() {
  auto& clients = committer->hdlr->clients;
  EndPoints endpoints;
  while(committer->hdlr->valid() &&
        (endpoints = clients->brokers.get_endpoints(_bkr_idx)).empty()) {
    SWC_LOG(LOG_ERROR, "Broker hosts cfg 'swc.bkr.host' is empty, waiting!");
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  if(endpoints.empty()) {
    handle_no_conn();
    return false;
  }
  clients->get_bkr_queue(endpoints)->put(req());
  return true;
}

void Committer_CellsUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::CellsUpdateRsp rsp_params(ev->error);
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
  committer->committed(req(), rsp_params, buffer);
}

}}}}}
