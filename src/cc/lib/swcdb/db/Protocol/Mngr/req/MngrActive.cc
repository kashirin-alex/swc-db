/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


MngrActive::MngrActive(const SWC::client::Clients::Ptr& clients,
                       const uint8_t& role, const cid_t& cid,
                       const DispatchHandler::Ptr& hdlr, uint32_t timeout_ms)
                      : client::ConnQueue::ReqBase(
                          false,
                          Buffers::make(
                            Params::MngrActiveReq(role, cid),
                            0,
                            MNGR_ACTIVE, timeout_ms
                          )
                        ),
                        clients(clients),
                        role(role), cid(cid), hdlr(hdlr), nxt(0),
                        timer(asio::high_resolution_timer(
                          clients->get_mngr_io()->executor())),
                        timeout_ms(timeout_ms) {
}

void MngrActive::run_within(uint32_t t_ms) {
  if(!hdlr->valid() ||
     clients->stopping() ||
     !clients->get_mngr_io()->running) {
    hdlr->run();
    return;
  }
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
  SWC_LOGF(LOG_DEBUG, "MngrActive(role=%d cid=%lu) no-conn", role, cid);
  if(!hdlr->valid() ||
     clients->stopping() ||
     !clients->get_mngr_io()->running) {
    hdlr->run();
  } else if(hosts.size() == ++nxt) {
    nxt = 0;
    hosts.clear();
    run_within(1000);
  } else {
    run();
  }
}

bool MngrActive::run() {
  if(hosts.empty()) {
    clients->managers.groups->hosts(role, cid, hosts, group_host);
    if(hosts.empty()) {
      SWC_LOGF(LOG_WARN, "Empty cfg of mngr.host for role=%d cid=%lu",
               role, cid);
      run_within(5000);
      return false;
    }
  }
  clients->get_mngr_queue(hosts.at(nxt))->put(req());
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
        group_host.endpoints = std::move(params.endpoints);
        clients->managers.groups->add(std::move(group_host));
        hdlr->run();
        return;
      }

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }

  run_within(500);
}


}}}}}
