/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/ClientConnQueues.h"

namespace SWC { namespace Comm { namespace client {


Host::~Host() {
  stop();
}

void Host::close_issued() {
  queues->remove(endpoints);
}

bool Host::connect() {
  queues->service->get_connection(
    endpoints,
    [ptr=shared_from_this()] (const ConnHandlerPtr& conn){ptr->set(conn);},
    std::chrono::milliseconds(queues->cfg_conn_timeout->get()),
    queues->cfg_conn_probes->get(),
    bool(cfg_keepalive_ms)
  );
  return true;
}



void ConnQueues::print(std::ostream& out) {
  out << "ConnQueues: ";
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& host : *this) {
    host->print(out);
    out << '\n';
  }
}

Host::Ptr ConnQueues::get(const EndPoints& endpoints){
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& host : *this) {
    if(has_endpoint(host->endpoints, endpoints))
      return host;
  }
  return emplace_back(new Host(
    shared_from_this(), endpoints, cfg_keepalive_ms, cfg_again_delay_ms));
}

void ConnQueues::remove(const EndPoints& endpoints) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    if(has_endpoint((*it)->endpoints, endpoints)) {
      erase(it);
      break;
    }
  }
}

void ConnQueues::stop() {
  for(;;) {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = cbegin();
    if(it == cend())
      break;
    (*it)->stop();
    erase(it);
  }
  service->stop();
}


}}}
