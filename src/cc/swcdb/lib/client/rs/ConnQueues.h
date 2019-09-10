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

  Host(const EndPoints& endpoints, 
       const gInt32tPtr timeout, const gInt32tPtr probes, 
       const bool persistent)
      : endpoints(endpoints),  
        cfg_conn_timeout(timeout), cfg_conn_probes(probes), 
        Protocol::Req::ConnQueue(persistent) {}

  virtual ~Host(){
    stop();  
  }
  
  bool connect() override {
    Env::Clients::get()->rs_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()] (client::ClientConPtr conn){ptr->set(conn);},
      std::chrono::milliseconds(cfg_conn_timeout->get()), 
      cfg_conn_probes->get(),
      !m_persistent
    );
    return true;
  }

  void close_issued() override;

  private:
  
  const gInt32tPtr  cfg_conn_timeout;
  const gInt32tPtr  cfg_conn_probes;
};


class ConnQueues {

  public:

  ConnQueues(const gInt32tPtr timeout, const gInt32tPtr probes, 
             bool persistent=true)
            : cfg_conn_timeout(timeout), cfg_conn_probes(probes), 
              m_persistent(persistent) {
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
      endpoints,
      cfg_conn_timeout,
      cfg_conn_probes,
      m_persistent
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
  const bool              m_persistent;

  const gInt32tPtr    cfg_conn_timeout;
  const gInt32tPtr    cfg_conn_probes;
  
};



void Host::close_issued() {
  Env::Clients::get()->rs->remove(endpoints);
}


}}}

#endif // swc_client_rs_ConnQueues_h