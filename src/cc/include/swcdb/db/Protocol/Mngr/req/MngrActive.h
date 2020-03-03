
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_mngr_req_MngrActive_h
#define swc_lib_db_protocol_mngr_req_MngrActive_h

#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class MngrActive : public Common::Req::ConnQueue::ReqBase {
  public:

  const int64_t cid;

  MngrActive(int64_t cid, DispatchHandler::Ptr hdlr, 
             uint32_t timeout_ms=60000)
            : Common::Req::ConnQueue::ReqBase(false), 
              cid(cid), hdlr(hdlr), timeout_ms(timeout_ms), nxt(0),
              timer(asio::high_resolution_timer(
                *Env::Clients::get()->mngr_service->io().get())) {
    cbp = CommBuf::make(Params::MngrActiveReq(cid, cid));
    cbp->header.set(MNGR_ACTIVE, timeout_ms);
  }

  virtual ~MngrActive(){ }

  void run_within(uint32_t t_ms = 1000) {
    timer.cancel();
    timer.expires_from_now(std::chrono::milliseconds(t_ms));
    timer.async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->run();
        }
      }
    );
  }

  void handle_no_conn() override { 
    if(hosts.size() == ++nxt) {
      nxt = 0;
      hosts.clear();
      run_within(1000);
      return;
    }
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(Env::IoCtx::stopping())
      return false;

    if(hosts.empty()) {
      Env::Clients::get()->mngrs_groups->hosts(cid, hosts, group_host);
      if(hosts.empty()) {
        SWC_LOGF(LOG_WARN, "Empty cfg of mngr.host for cid=%d", cid);
        run_within(5000);
        return false;
      }
    }
    
    Env::Clients::get()->mngr->get(hosts.at(nxt))->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    
    // SWC_LOGF(LOG_DEBUG, " handle: %s", ev->to_str().c_str());

    if(ev->error == Error::OK && ev->header.command == MNGR_ACTIVE) {

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

      } catch (Exception &e) {
        SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      }
    }

    run_within(500);
  }

  private:
  DispatchHandler::Ptr            hdlr;
  int                             nxt;
  client::Mngr::Hosts             hosts;
  client::Mngr::Groups::GroupHost group_host;
  asio::high_resolution_timer     timer;

  protected:
  const uint32_t  timeout_ms;
};

}}}}

#endif // swc_lib_db_protocol_mngr_req_MngrActive_h
