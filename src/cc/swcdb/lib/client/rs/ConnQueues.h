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

  Host(const EndPoints& endpoints)
      : endpoints(endpoints),
        cfg_rs_conn_timeout(Env::Config::settings()->get_ptr<gInt32t>(
          "swc.client.RS.connection.timeout")),
        cfg_rs_conn_probes(Env::Config::settings()->get_ptr<gInt32t>(
          "swc.client.RS.connection.probes")) {}

  virtual ~Host(){
    stop(); 
  }
  
  bool connect() override {
    Env::Clients::get()->rs_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()] (client::ClientConPtr conn){ptr->set(conn);},
      std::chrono::milliseconds(cfg_rs_conn_timeout->get()), 
      cfg_rs_conn_probes->get(),
      true
    );
    return true;
  }

  private:
  
  const gInt32tPtr cfg_rs_conn_timeout;
  const gInt32tPtr cfg_rs_conn_probes;
};


class ConnQueues {

  public:

  ConnQueues() {}

  virtual ~ConnQueues() { }

  std::string to_string(){
    std::string s("Rs ConnQueues: ");
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
    auto host = std::make_shared<Host>(endpoints);
    m_hosts.push_back(host);
    return host;
  }


  private:

  std::mutex              m_mutex;
  std::vector<Host::Ptr>  m_hosts;
};



}}}

#endif // swc_client_rs_ConnQueues_h