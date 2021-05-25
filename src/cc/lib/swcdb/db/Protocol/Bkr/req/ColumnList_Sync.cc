/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/ColumnList_Sync.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


SWC_SHOULD_INLINE
void ColumnList_Sync::request(
                      const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnListReq& params,
                      int& err, std::vector<DB::Schema::Ptr>& schemas,
                      const uint32_t timeout) {
  auto req = std::make_shared<ColumnList_Sync>(
    clients, params, err, schemas, timeout);
  auto res = req->await.get_future();
  req->run();
  res.get();
}


ColumnList_Sync::ColumnList_Sync(
                      const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnListReq& params,
                      int& err, std::vector<DB::Schema::Ptr>& schemas,
                      const uint32_t timeout)
                      : client::ConnQueue::ReqBase(
                          false,
                          Buffers::make(params, 0, COLUMN_LIST, timeout)
                        ),
                        clients(clients), err(err), schemas(schemas),
                        bkr_idx(0) {
}

void ColumnList_Sync::handle_no_conn() {
  if(clients->stopping()) {
    err = Error::CLIENT_STOPPING;
    await.set_value();
  } else {
    ++bkr_idx;
    run();
  }
}

bool ColumnList_Sync::run() {
  EndPoints endpoints;
  while(!clients->stopping() &&
        (endpoints = clients->brokers.get_endpoints(bkr_idx)).empty()) {
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

void ColumnList_Sync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      Mngr::Params::ColumnListRsp rsp;
      rsp.decode(&ptr, &remain);
      schemas = std::move(rsp.schemas);

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

  await.set_value();
}


}}}}}
