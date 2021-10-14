/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void MngrActive::run_within(uint32_t t_ms) {
  if(!hdlr->valid() ||
     clients->stopping() ||
     !clients->get_mngr_io()->running) {
    hdlr->run();
    return;
  }
  timer.cancel();
  timer.expires_after(std::chrono::milliseconds(t_ms));
  struct TimerTask {
    DispatchHandler::Ptr ptr;
    SWC_CAN_INLINE
    TimerTask(DispatchHandler::Ptr&& a_ptr) noexcept
              : ptr(std::move(a_ptr)) { }
    ~TimerTask() noexcept { }
    void operator()(const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted)
        ptr->run();
    }
  };
  timer.async_wait(TimerTask(shared_from_this()));
}

void MngrActive::handle_no_conn() {
  SWC_LOGF(LOG_DEBUG,
    "MngrActive(role=%d cid=" SWC_FMT_LU " req=%s) no-conn",
    role, cid, Core::type_name(*hdlr.get()));
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
      SWC_LOGF(LOG_WARN,
        "Empty cfg of mngr.host for role=%d cid=" SWC_FMT_LU " req=%s",
        role, cid, Core::type_name(*hdlr.get()));
      run_within(5000);
      return false;
    }
    if(nxt >= hosts.size())
      nxt = 0;
  }
  clients->get_mngr_queue(hosts[nxt])->put(req());
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

      if(params.endpoints.size()) {
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
