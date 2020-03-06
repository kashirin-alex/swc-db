/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/client/ConnQueues.h"

namespace SWC { namespace client {

Host::Host(const ConnQueuesPtr queues, const EndPoints& endpoints, 
           const Property::V_GINT32::Ptr keepalive_ms, 
           const Property::V_GINT32::Ptr again_delay_ms)
          : queues(queues), endpoints(endpoints), 
            Protocol::Common::Req::ConnQueue(keepalive_ms, again_delay_ms) {
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
    [ptr=shared_from_this()] (ConnHandlerPtr conn){ptr->set(conn);},
    std::chrono::milliseconds(queues->cfg_conn_timeout->get()), 
    queues->cfg_conn_probes->get(),
    cfg_keepalive_ms != nullptr
  );
  return true;  
}

ConnQueues::ConnQueues(const Serialized::Ptr service, 
                       const Property::V_GINT32::Ptr timeout,
                       const Property::V_GINT32::Ptr probes, 
                       const Property::V_GINT32::Ptr keepalive_ms,
                       const Property::V_GINT32::Ptr again_delay_ms)
                      : service(service),
                        cfg_conn_timeout(timeout),
                        cfg_conn_probes(probes), 
                        cfg_keepalive_ms(keepalive_ms),
                        cfg_again_delay_ms(again_delay_ms) {
}

ConnQueues::~ConnQueues() { }

std::string ConnQueues::to_string() {
  std::string s("ConnQueues: ");
  std::lock_guard<std::mutex> lock(m_mutex);

  for(auto& host : m_hosts){
    s.append(host->to_string());
    s.append("\n");
  }
  return s;
}

Host::Ptr ConnQueues::get(const EndPoints& endpoints){
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto& host : m_hosts){
    if(has_endpoint(host->endpoints, endpoints))
      return host;
  }
  return m_hosts.emplace_back(
    new Host(
      shared_from_this(), endpoints, cfg_keepalive_ms, cfg_again_delay_ms));
}

void ConnQueues::remove(const EndPoints& endpoints) {
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto it=m_hosts.begin(); it<m_hosts.end(); it++){

    if(has_endpoint((*it)->endpoints, endpoints)) {
      m_hosts.erase(it);
      break;
    }
  }
}

}}
