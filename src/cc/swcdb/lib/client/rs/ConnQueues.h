/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_rs_ConnQueues_h
#define swc_client_rs_ConnQueues_h

namespace SWC { namespace client { namespace Rs {

class Host : public Protocol::Req::ConnQueue  {
  public:
  typedef std::shared_ptr<Host> Ptr;

  const EndPoints   endpoints;

  Host(const ConnQueuesPtr queues, const EndPoints& endpoints, 
       const gInt32tPtr timeout, const gInt32tPtr probes, 
       const gInt32tPtr keepalive_ms)
      : queues(queues), endpoints(endpoints), 
        cfg_conn_timeout(timeout), cfg_conn_probes(probes), 
        Protocol::Req::ConnQueue(keepalive_ms) {
  }

  virtual ~Host(){
    stop();  
  }
  
  bool connect() override {
    Env::Clients::get()->rs_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()] (client::ClientConPtr conn){ptr->set(conn);},
      std::chrono::milliseconds(cfg_conn_timeout->get()), 
      cfg_conn_probes->get(),
      cfg_keepalive_ms != nullptr
    );
    return true;
  }

  void close_issued() override;

  protected:
  const ConnQueuesPtr queues;
  const gInt32tPtr    cfg_conn_timeout;
  const gInt32tPtr    cfg_conn_probes;
};


class ConnQueues : public std::enable_shared_from_this<ConnQueues> {

  public:

  ConnQueues(const gInt32tPtr timeout, const gInt32tPtr probes, 
             const gInt32tPtr keepalive_ms)
            : cfg_conn_timeout(timeout), cfg_conn_probes(probes), 
              cfg_keepalive_ms(keepalive_ms) {
  }

  virtual ~ConnQueues() { }

  std::string to_string(){
    std::string s("ConnQueues: ");
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto& host : m_hosts){
      s.append(host->to_string());
      s.append("\n");
    }
    return s;
  }

  Host::Ptr get(EndPoints& endpoints){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& host : m_hosts){
      if(has_endpoint(host->endpoints, endpoints))
        return host;
    }

    auto host = std::make_shared<Host>(
      shared_from_this(), endpoints, 
      cfg_conn_timeout, cfg_conn_probes, cfg_keepalive_ms
    );

    m_hosts.push_back(host);
    return host;
  }

  void remove(const EndPoints& endpoints) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto it=m_hosts.begin(); it<m_hosts.end(); it++){

      if(has_endpoint((*it)->endpoints, endpoints)) {
        m_hosts.erase(it);
        break;
      }
    }
  }

  private:

  std::mutex              m_mutex;
  std::vector<Host::Ptr>  m_hosts;

  const gInt32tPtr    cfg_conn_timeout;
  const gInt32tPtr    cfg_conn_probes;
  const gInt32tPtr    cfg_keepalive_ms;
  
};



void Host::close_issued() {
  queues->remove(endpoints);
}


}}}

#endif // swc_client_rs_ConnQueues_h