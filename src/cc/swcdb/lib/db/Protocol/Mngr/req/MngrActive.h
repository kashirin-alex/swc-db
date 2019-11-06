
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_mngr_req_MngrActive_h
#define swc_lib_db_protocol_mngr_req_MngrActive_h

#include "../params/MngrActive.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class MngrActive : public Common::Req::ConnQueue::ReqBase {
  public:

  const int64_t cid;

  MngrActive(int64_t cid, DispatchHandlerPtr hdlr, 
             uint32_t timeout_ms=60000)
            : Common::Req::ConnQueue::ReqBase(false), 
              cid(cid), hdlr(hdlr), timeout_ms(timeout_ms), nxt(0) {
    cbp = CommBuf::make(Params::MngrActiveReq(cid, cid));
    cbp->header.set(MNGR_ACTIVE, timeout_ms);
  }

  virtual ~MngrActive(){ }

  void handle_no_conn() override { 
    if(hosts.size() == ++nxt) {
      nxt = 0;
      hosts.clear();
      run_within(Env::Clients::get()->mngr_service->io(), 1000);
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
        HT_WARNF("Empty cfg of mngr.host for cid=%d", cid);
        run_within(Env::Clients::get()->mngr_service->io(), 5000);
        return false;
      }
    }
    
    Env::Clients::get()->mngr->get(hosts.at(nxt))->put(req());
    return true;
  }

  virtual void handle(ConnHandlerPtr conn, Event::Ptr &ev) {
    
    // HT_DEBUGF(" handle: %s", ev->to_str().c_str());

    if(ev->error == Error::OK && ev->header.command == MNGR_ACTIVE){

      try {
        const uint8_t *ptr = ev->data.base;
        size_t remain = ev->data.size;

        Params::MngrActiveRsp params;
        params.decode(&ptr, &remain);
        
        if(params.available && params.endpoints.size() > 0){
          group_host.endpoints = params.endpoints; 
          Env::Clients::get()->mngrs_groups->add(group_host);
          hdlr->run();
          return;
        }

      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
      }
    }

    run_within(conn->io_ctx, 500);
  }

  private:
  DispatchHandlerPtr      hdlr;
  int                     nxt;
  client::Mngr::Hosts     hosts;
  client::Mngr::Groups::GroupHost group_host;

  protected:
  const uint32_t  timeout_ms;
};

}}}}

#endif // swc_lib_db_protocol_mngr_req_MngrActive_h
