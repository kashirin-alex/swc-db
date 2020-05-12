/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include <chrono>
#include <string>
#include <queue>
#include <unordered_map>

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/SerializedClient.h"


namespace SWC { namespace client {


ServerConnections::ServerConnections(const std::string& srv_name, 
                                     const EndPoint& endpoint,
                                     IOCtxPtr ioctx, AppContext::Ptr ctx,
                                     ConfigSSL* ssl_cfg)
                                    : m_srv_name(srv_name), 
                                      m_endpoint(endpoint), 
                                      m_ioctx(ioctx), m_ctx(ctx), 
                                      m_ssl_cfg(ssl_cfg) {
}

ServerConnections::~ServerConnections() { }

void ServerConnections::reusable(ConnHandlerPtr &conn, bool preserve) {
  for(;;){
    Mutex::scope lock(m_mutex);
    if(m_conns.empty())
      return;
    conn = m_conns.front();
    if(conn->is_open()){
      if(!preserve)
        m_conns.pop();
      return; 
    }
    m_conns.pop();
    conn = nullptr;
  }
  // else
  //  SWC_LOGF(LOG_DEBUG, "Reusing connection: %s, %s", 
  //             m_srv_name.c_str(), to_string(conn).c_str());
}

void ServerConnections::connection(ConnHandlerPtr &conn,  
                                   std::chrono::milliseconds timeout, 
                                   bool preserve) {

  SWC_LOGF(LOG_DEBUG, "Connecting Sync: %s, addr=[%s]:%d %s", 
           m_srv_name.c_str(),  
           m_endpoint.address().to_string().c_str(), m_endpoint.port(),
           m_ssl_cfg ? "SECURE" : "PLAIN");
    
  asio::ip::tcp::socket sock(*m_ioctx.get());
  asio::error_code ec;
  sock.open(m_endpoint.protocol(), ec);
  if(ec || !sock.is_open())
    return;

  sock.set_option(asio::ip::tcp::no_delay(true));

  sock.connect(m_endpoint, ec);
  if(ec || !sock.is_open())
    return;
  //sock.non_blocking(true);

  if(m_ssl_cfg) {
    conn = m_ssl_cfg->make_client(m_ctx, sock, ec);
    if(ec || !conn->is_open())
      return;
  } else {
    conn = std::make_shared<ConnHandlerPlain>(m_ctx, sock);
    conn->new_connection();
  }
  if(preserve)
    put_back(conn);
  // SWC_LOGF(LOG_DEBUG, "New connection: %s, %s", 
  //          m_srv_name.c_str(), to_string(conn).c_str());
}

void ServerConnections::connection(std::chrono::milliseconds timeout, 
                                   NewCb_t cb, bool preserve) {

  SWC_LOGF(LOG_DEBUG, "Connecting Async: %s, addr=[%s]:%d %s", 
           m_srv_name.c_str(), 
           m_endpoint.address().to_string().c_str(), m_endpoint.port(),
           m_ssl_cfg ? "SECURE" : "PLAIN");
    
  auto sock = std::make_shared<asio::ip::tcp::socket>(*m_ioctx.get());
  sock->async_connect(
    m_endpoint, 
    [sock, cb, preserve, ptr=shared_from_this()]
    (const std::error_code& ec) {
      if(ec || !sock->is_open()){
        cb(nullptr);
        return;
      }
      sock->set_option(asio::ip::tcp::no_delay(true));
      //sock->non_blocking(true);

      if(ptr->m_ssl_cfg) {
        ptr->m_ssl_cfg->make_client(
          ptr->m_ctx, *sock.get(),
          [cb, preserve, ptr]
          (ConnHandlerPtr conn, const std::error_code& ec) {
            if(ec || !conn->is_open()) {
              cb(nullptr);
            } else {
              if(preserve)
                ptr->put_back(conn);
              cb(conn);
            }
          }
        );
        return;
      }
      auto conn = 
        std::make_shared<ConnHandlerPlain>(ptr->m_ctx, *sock.get());
      conn->new_connection();
      if(preserve)
        ptr->put_back(conn);
      cb(conn);
    }
  );       
}

void ServerConnections::put_back(ConnHandlerPtr conn){
  Mutex::scope lock(m_mutex);
  m_conns.push(conn);
}
  
bool ServerConnections::empty() {
  Mutex::scope lock(m_mutex);
  return m_conns.empty();
}

void ServerConnections::close_all(){
  for(ConnHandlerPtr conn;;) {
    {
      Mutex::scope lock(m_mutex);
      if(m_conns.empty())
        break;
      conn = m_conns.front();
      m_conns.pop();
    }
    conn->close();
  }
}


Serialized::Serialized(const std::string& srv_name, IOCtxPtr ioctx, 
                       AppContext::Ptr ctx)
            : m_srv_name(srv_name), m_ioctx(ioctx), m_ctx(ctx),
              m_use_ssl(Env::Config::settings()->get_bool("swc.comm.ssl")),
              m_ssl_cfg(m_use_ssl ? new ConfigSSL() : nullptr), 
              m_run(true) {
  SWC_LOGF(LOG_INFO, "Init: %s", m_srv_name.c_str());
}

ServerConnections::Ptr Serialized::get_srv(EndPoint endpoint) {
  size_t hash = endpoint_hash(endpoint);
  Mutex::scope lock(m_mutex);

  auto it = m_srv_conns.find(hash);
  if(it == m_srv_conns.end())
    it = m_srv_conns.emplace(
      hash, 
      std::make_shared<ServerConnections>(
        m_srv_name, endpoint, m_ioctx, m_ctx,
        m_use_ssl && m_ssl_cfg->need_ssl(endpoint) ? m_ssl_cfg : nullptr
      )
    ).first;
    
  return it->second;
}

ConnHandlerPtr Serialized::get_connection(
      const EndPoints& endpoints, 
      std::chrono::milliseconds timeout, uint32_t probes, bool preserve) {
    
  ConnHandlerPtr conn = nullptr;
  if(endpoints.empty()){
    SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints", 
                        m_srv_name.c_str());
    return conn;
  }
    
  ServerConnections::Ptr srv;
  uint32_t tries = probes;
  do {

    for(auto& endpoint : endpoints){
      srv = get_srv(endpoint);
      srv->reusable(conn, preserve);
      if(conn != nullptr)
        return conn;
          
      srv->connection(conn, timeout, preserve);
      if(conn != nullptr)
        return conn;
    }
    SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%d", 
                         m_srv_name.c_str(), tries);
      
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // ? cfg-setting

  } while (m_run.load() && (!probes || --tries));

  return conn;
}

void Serialized::get_connection(
      const EndPoints& endpoints, ServerConnections::NewCb_t cb,
      std::chrono::milliseconds timeout, uint32_t probes, bool preserve) {
    
  if(endpoints.empty()){
    SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints", 
                        m_srv_name.c_str());
    cb(nullptr);
    return;
  }

  get_connection(endpoints, cb, timeout, probes, probes, 0, preserve);
}
  
void Serialized::get_connection(
      const EndPoints& endpoints, ServerConnections::NewCb_t cb,
      std::chrono::milliseconds timeout, uint32_t probes, uint32_t tries, 
      int next, bool preserve) {
          
  if(next == endpoints.size())
    next = 0;

  ServerConnections::Ptr srv = get_srv(endpoints.at(next));
  ConnHandlerPtr conn = nullptr;
  srv->reusable(conn, preserve);
  if(conn != nullptr || (probes && !tries)) {
    cb(conn);
    return;
  }
    
  ++next;
  SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%d", m_srv_name.c_str(), tries);
  srv->connection(timeout, 
    [endpoints, cb, timeout, probes, tries, next, preserve, ptr=shared_from_this()]
    (ConnHandlerPtr conn){
      if(!ptr->m_run.load() || (conn != nullptr && conn->is_open())){
        cb(conn);
        return;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // ? cfg-setting

      ptr->get_connection(
        endpoints, cb, timeout, 
        probes, next == endpoints.size() ? tries-1 : tries, 
        next, 
        preserve
      );
    },
    preserve
  );
}

void Serialized::preserve(ConnHandlerPtr conn) {
  if(!conn->is_open()) {
    conn->do_close();
    return;
  }
  size_t hash = conn->endpoint_remote_hash();

  Mutex::scope lock(m_mutex);
  auto it = m_srv_conns.find(hash);
  if(it != m_srv_conns.end())
    (*it).second->put_back(conn);
}

void Serialized::close(ConnHandlerPtr conn){
  size_t hash = conn->endpoint_remote_hash();
  conn->do_close();

  Mutex::scope lock(m_mutex);
  auto it = m_srv_conns.find(hash);
  if(it != m_srv_conns.end() && (*it).second->empty())
    m_srv_conns.erase(it);
}

IOCtxPtr Serialized::io() {
  return m_ioctx; 
}             
  
std::string Serialized::to_str(ConnHandlerPtr conn) {
  std::string s(m_srv_name);
  s.append(" ");
  s.append(conn->to_string());
  return s;
}
  
void Serialized::stop() {
  m_run.store(false);

  Map::iterator it;
  for(ServerConnections::Ptr srv;;) {
    {
      Mutex::scope lock(m_mutex);
      if((it = m_srv_conns.begin()) == m_srv_conns.end())
        break;
      srv = it->second;
      m_srv_conns.erase(it);
    }
    srv->close_all();
  }
  SWC_LOGF(LOG_INFO, "Stop: %s", m_srv_name.c_str());
}

Serialized::~Serialized() { 
  if(m_ssl_cfg)
    delete m_ssl_cfg;
}



}}
