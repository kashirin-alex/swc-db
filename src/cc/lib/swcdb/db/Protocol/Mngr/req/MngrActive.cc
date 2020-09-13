
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {



MngrActive::Ptr MngrActive::make(const cid_t& cid, 
                                 const DispatchHandler::Ptr& hdlr,
                                 uint32_t timeout_ms) {
  return std::make_shared<MngrActive>(
    Types::MngrRole::COLUMNS, cid, hdlr, timeout_ms);
}

MngrActive::Ptr MngrActive::make(const uint8_t& role, 
                                 const DispatchHandler::Ptr& hdlr,
                                 uint32_t timeout_ms) {
  return std::make_shared<MngrActive>(
    role, DB::Schema::NO_CID, hdlr, timeout_ms);
}

MngrActive::MngrActive(const uint8_t& role, const cid_t& cid, 
                       const DispatchHandler::Ptr& hdlr, uint32_t timeout_ms)
                      : client::ConnQueue::ReqBase(false), 
                        role(role), cid(cid), hdlr(hdlr), nxt(0),
                        timer(asio::high_resolution_timer(
                          *Env::Clients::get()->mngr->service->io().get())),
                        timeout_ms(timeout_ms) {
  cbp = CommBuf::make(Params::MngrActiveReq(role, cid));
  cbp->header.set(MNGR_ACTIVE, timeout_ms);
}

MngrActive::~MngrActive() { }

void MngrActive::run_within(uint32_t t_ms) {
  timer.cancel();
  timer.expires_after(std::chrono::milliseconds(t_ms));
  timer.async_wait(
    [ptr=shared_from_this()](const asio::error_code& ec) {
      if (ec != asio::error::operation_aborted){
        ptr->run();
      }
    }
  );
}

void MngrActive::handle_no_conn() {
  if(hosts.size() == ++nxt) {
    nxt = 0;
    hosts.clear();
    run_within(1000);
    return;
  }
  run();
}

bool MngrActive::run() {
  if(Env::IoCtx::stopping())
    return false;

  if(hosts.empty()) {
    Env::Clients::get()->mngrs_groups->hosts(role, cid, hosts, group_host);
    if(hosts.empty()) {
      SWC_LOGF(LOG_WARN, "Empty cfg of mngr.host for role=%d cid=%lu", 
               role, cid);
      run_within(5000);
      return false;
    }
  }
  
  Env::Clients::get()->mngr->get(hosts.at(nxt))->put(req());
  return true;
}

void MngrActive::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  // SWC_LOGF(LOG_DEBUG, " handle: %s", ev->to_str().c_str());

  if(!ev->error) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      Params::MngrActiveRsp params;
      params.decode(&ptr, &remain);
      
      if(params.available && params.endpoints.size()) {
        group_host.endpoints = params.endpoints; 
        Env::Clients::get()->mngrs_groups->add(group_host);
        hdlr->run();
        return;
      }

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }

  run_within(500);
}


}}}}