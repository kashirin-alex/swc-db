/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <chrono>
#include <string>
#include <queue>
#include <unordered_map>

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/SerializedClient.h"


namespace SWC { namespace client {

  
asio::ssl::context SSL_Context::create() {
  asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
  ssl_ctx.set_options(
      asio::ssl::context::default_workarounds
    | asio::ssl::context::no_compression
    | asio::ssl::context::no_sslv2
    | asio::ssl::context::no_sslv3
    | asio::ssl::context::no_tlsv1
    | asio::ssl::context::no_tlsv1_1
  );
  ssl_ctx.set_verify_mode(
    asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

  if(ca.empty()) {
    ssl_ctx.set_default_verify_paths();
  } else {
    //ssl_ctx.load_verify_file(ca); problems
    ssl_ctx.add_certificate_authority(
      asio::const_buffer(ca.c_str(), ca.length()));
  }
  return ssl_ctx;

}
void SSL_Context::load_ca(const std::string& ca_filepath) {
  std::ifstream ifs(ca_filepath, std::ifstream::in);
  std::string line;
  ca.clear();
  while(getline(ifs, line))
    ca.append(line);
  ifs.close();
}



ServerConnections::ServerConnections(const std::string& srv_name, 
                                     const EndPoint& endpoint,
                                     IOCtxPtr ioctx, AppContext::Ptr ctx,
                                     SSL_Context* ssl_ctx)
                                    : m_srv_name(srv_name), 
                                      m_endpoint(endpoint), 
                                      m_ioctx(ioctx), m_ctx(ctx), 
                                      m_ssl_ctx(ssl_ctx) {
}

ServerConnections::~ServerConnections() { }

void ServerConnections::reusable(ConnHandlerPtr &conn, bool preserve) {
  for(;;){
    std::lock_guard<std::mutex> lock(m_mutex);
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
           m_ssl_ctx ? "SECURE" : "PLAIN");
    
  asio::ip::tcp::socket sock(*m_ioctx.get());
  asio::error_code ec;
  sock.open(m_endpoint.protocol(), ec);
  if(ec || !sock.is_open())
    return;

  sock.connect(m_endpoint, ec);
  if(ec || !sock.is_open())
    return;

  if(m_ssl_ctx) {
    auto ssl_ctx = m_ssl_ctx->create();
    auto ssl_conn = std::make_shared<ConnHandlerSSL>(m_ctx, ssl_ctx, sock);
    conn = ssl_conn;
    ssl_conn->handshake_client(ec, m_ssl_ctx->subject_name);
    if(ec || !ssl_conn->is_open())
      return;
  } else
    conn = std::make_shared<ConnHandlerPlain>(m_ctx, sock);

  conn->new_connection();
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
           m_ssl_ctx ? "SECURE" : "PLAIN");
    
  auto sock = std::make_shared<asio::ip::tcp::socket>(*m_ioctx.get());
  sock->async_connect(m_endpoint, 
    [sock, cb, preserve, ptr=shared_from_this()]
    (const std::error_code& ec) {
      if(ec || !sock->is_open()){
        cb(nullptr);
        return;
      }
      if(ptr->m_ssl_ctx) {
        auto ssl_ctx =  ptr->m_ssl_ctx->create();
        auto conn = 
          std::make_shared<ConnHandlerSSL>(ptr->m_ctx, ssl_ctx, *sock.get());
        conn->new_connection();
        conn->handshake_client(
          [conn, cb, preserve, ptr](const std::error_code& ec) {
            if(ec || !conn->is_open()) {
              cb(nullptr);
            } else {
              if(preserve)
                ptr->put_back(conn);
              cb(conn);
            }
          },
          ptr->m_ssl_ctx->subject_name
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
  std::lock_guard<std::mutex> lock(m_mutex);
  m_conns.push(conn);
}
  
const bool ServerConnections::empty() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_conns.empty();
}

void ServerConnections::close_all(){
  std::lock_guard<std::mutex> lock(m_mutex);
  while(!m_conns.empty()){
    m_conns.front()->close();
    m_conns.pop();
  }
}


Serialized::Serialized(const std::string& srv_name, IOCtxPtr ioctx, 
                       AppContext::Ptr ctx)
            : m_srv_name(srv_name), m_ioctx(ioctx), m_ctx(ctx),
              m_use_ssl(Env::Config::settings()->get<bool>("swc.comm.ssl")),
              m_ssl_ctx(m_use_ssl ? new SSL_Context() : nullptr), 
              m_run(true) {

  if(m_use_ssl) {
    asio::error_code ec;
    Resolver::get_networks(
      Env::Config::settings()->get<Strings>("swc.comm.ssl.secure.network"),
      m_ssl_ctx->nets_v4,
      m_ssl_ctx->nets_v6,
      ec
    );
    if(ec)
      SWC_THROWF(Error::CONFIG_BAD_VALUE,
                "Bad Network in swc.comm.ssl.secure.network error(%s)",
                ec.message().c_str());
    
    m_ssl_ctx->subject_name = 
      Env::Config::settings()->has("swc.comm.ssl.subject_name")
      ? Env::Config::settings()->get<std::string>("swc.comm.ssl.subject_name")
      : "";

    if(Env::Config::settings()->has("swc.comm.ssl.ca")) {
      auto ca = Env::Config::settings()->get<std::string>("swc.comm.ssl.ca");
      if(ca.front() != '.' && ca.front() != '/')
        ca = Env::Config::settings()->get<std::string>("swc.cfg.path") + ca;
      m_ssl_ctx->load_ca(ca);
    }
  }

  SWC_LOGF(LOG_INFO, "Init: %s", m_srv_name.c_str());
}

ServerConnections::Ptr Serialized::get_srv(EndPoint endpoint) {
  size_t hash = endpoint_hash(endpoint);
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_srv_conns.find(hash);
  if(it != m_srv_conns.end())
    return (*it).second;

  bool use_ssl = m_use_ssl &&
    !Resolver::is_network(endpoint, m_ssl_ctx->nets_v4, m_ssl_ctx->nets_v6);

  auto srv = std::make_shared<ServerConnections>(
    m_srv_name, endpoint, m_ioctx, m_ctx, 
    use_ssl ? m_ssl_ctx : nullptr
  );
  m_srv_conns.insert(std::make_pair(hash, srv));
  return srv;
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

  ServerConnections::Ptr srv = get_srv(endpoints.at(next++));
  ConnHandlerPtr conn = nullptr;
  srv->reusable(conn, preserve);
  if(conn != nullptr || (probes && !tries)) {
    cb(conn);
    return;
  }
    
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
  size_t hash = conn->endpoint_remote_hash();

  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_srv_conns.find(hash);
  if(it != m_srv_conns.end())
    (*it).second->put_back(conn);
}

void Serialized::close(ConnHandlerPtr conn){
  size_t hash = conn->endpoint_remote_hash();
  conn->do_close();

  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_srv_conns.find(hash);
  if(it != m_srv_conns.end() && (*it).second->empty())
    m_srv_conns.erase(it);
}

IOCtxPtr Serialized::io() {
  return m_ioctx; 
}             
  
const std::string Serialized::to_str(ConnHandlerPtr conn) {
  std::string s(m_srv_name);
  s.append(" ");
  s.append(conn->to_string());
  return s;
}
  
void Serialized::stop() {
  m_run.store(false);
    
  std::lock_guard<std::mutex> lock(m_mutex);
  Map::iterator it;
  while((it = m_srv_conns.begin()) != m_srv_conns.end()){
    it->second->close_all();
    m_srv_conns.erase(it);
  }
  SWC_LOGF(LOG_INFO, "Stop: %s", m_srv_name.c_str());
}

Serialized::~Serialized() { 
  if(m_ssl_ctx)
    delete m_ssl_ctx;
}



}}
