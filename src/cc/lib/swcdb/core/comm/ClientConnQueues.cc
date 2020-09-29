/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/ClientConnQueues.h"

namespace SWC { namespace Comm { namespace client {

Host::Host(const ConnQueuesPtr queues, const EndPoints& endpoints, 
           const Config::Property::V_GINT32::Ptr keepalive_ms, 
           const Config::Property::V_GINT32::Ptr again_delay_ms)
          : ConnQueue(queues->service->io(), keepalive_ms, again_delay_ms),
            endpoints(endpoints), queues(queues) {
}

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
    cfg_keepalive_ms != nullptr
  );
  return true;  
}

ConnQueues::ConnQueues(const Serialized::Ptr service, 
                       const Config::Property::V_GINT32::Ptr timeout,
                       const Config::Property::V_GINT32::Ptr probes, 
                       const Config::Property::V_GINT32::Ptr keepalive_ms,
                       const Config::Property::V_GINT32::Ptr again_delay_ms)
                      : service(service),
                        cfg_conn_timeout(timeout),
                        cfg_conn_probes(probes), 
                        cfg_keepalive_ms(keepalive_ms),
                        cfg_again_delay_ms(again_delay_ms) {
}

ConnQueues::~ConnQueues() { }

void ConnQueues::print(std::ostream& out) {
  out << "ConnQueues: ";
  Mutex::scope lock(m_mutex);
  for(auto& host : *this) {
    host->print(out);
    out << '\n';
  }
}

Host::Ptr ConnQueues::get(const EndPoints& endpoints){
  Mutex::scope lock(m_mutex);
  for(auto& host : *this) {
    if(has_endpoint(host->endpoints, endpoints))
      return host;
  }
  return emplace_back(new Host(
    shared_from_this(), endpoints, cfg_keepalive_ms, cfg_again_delay_ms));
}

void ConnQueues::remove(const EndPoints& endpoints) {
  Mutex::scope lock(m_mutex);
  for(auto it=begin(); it<end(); ++it) {
    if(has_endpoint((*it)->endpoints, endpoints)) {
      erase(it);
      break;
    }
  }
}

void ConnQueues::stop() {
  for(;;) {
    Mutex::scope lock(m_mutex);
    auto it=begin();
    if(it == end())
      break;
    (*it)->stop();
    erase(it);
  }
  service->stop();
}


}}}
