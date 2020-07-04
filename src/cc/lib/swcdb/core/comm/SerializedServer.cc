/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include <unordered_map>

#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"


namespace SWC { namespace server {

Acceptor::Acceptor(asio::ip::tcp::acceptor& acceptor, 
                  AppContext::Ptr& app_ctx, bool is_plain)
                  : asio::ip::tcp::acceptor(std::move(acceptor)), 
                    m_app_ctx(app_ctx) {
  set_option(asio::ip::tcp::acceptor::reuse_address(true));
  //set_option(asio::ip::tcp::no_delay(true));
  //non_blocking(true);

  SWC_LOGF(
    LOG_INFO, "Listening On: [%s]:%d fd=%lu %s", 
    local_endpoint().address().to_string().c_str(), 
    local_endpoint().port(), 
    (size_t)native_handle(),
    is_plain ? "PLAIN" : "SECURE"
  );
  if(is_plain)
    do_accept();
}

void Acceptor::stop() {
  SWC_LOGF(LOG_INFO, "Stopping to Listen On: [%s]:%d fd=%lu", 
           local_endpoint().address().to_string().c_str(), 
           local_endpoint().port(), 
           (size_t)native_handle());

  if(is_open())
    close();
}

Acceptor::~Acceptor() { }

asio::ip::tcp::acceptor* Acceptor::sock() {
  return this;
}

void Acceptor::do_accept() {
  async_accept(
    [this](const std::error_code& ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != 125) 
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)", 
                    ec.value(), ec.message().c_str());
        return;
      }
      //new_sock.set_option(asio::ip::tcp::no_delay(true));
      //new_sock.non_blocking(true);
      auto conn = std::make_shared<ConnHandlerPlain>(m_app_ctx, new_sock);
      conn->new_connection();
      conn->accept_requests();

      do_accept();
    }
  );
}




AcceptorSSL::AcceptorSSL(asio::ip::tcp::acceptor& acceptor, 
                        AppContext::Ptr& app_ctx, ConfigSSL* ssl_cfg)
                        : Acceptor(acceptor, app_ctx, false),
                          m_ssl_cfg(ssl_cfg) {
  do_accept();
}

void AcceptorSSL::do_accept() {
  async_accept(
    [this](const std::error_code& ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != 125) 
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)", 
                    ec.value(), ec.message().c_str());
        return;
      }
      //new_sock.set_option(asio::ip::tcp::no_delay(true));
      //new_sock.non_blocking(true);
      m_ssl_cfg->make_server(m_app_ctx, new_sock);
      do_accept();
    }
  );
}

AcceptorSSL::~AcceptorSSL(){ }



SerializedServer::SerializedServer(
    std::string name, 
    uint32_t reactors, uint32_t workers,
    const std::string& port_cfg_name,
    AppContext::Ptr app_ctx
  ) : m_appname(name), m_run(true),
      m_ssl_cfg(Env::Config::settings()->get_bool("swc.comm.ssl")
                ? new ConfigSSL(false) : nullptr) {
    
  SWC_LOGF(LOG_INFO, "STARTING SERVER: %s, reactors=%d, workers=%d", 
           m_appname.c_str(), reactors, workers);

  auto settings = Env::Config::settings();

  Strings addrs = settings->has("addr") ? settings->get_strs("addr") : Strings();
  std::string host;
  if(settings->has("host"))
    host = host.append(settings->get_str("host"));
  else {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }
    
  std::vector<Network> nets;
  asio::error_code ec;
  Resolver::get_networks(
    settings->get_strs("swc.comm.network.priority"), nets, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "swc.comm.network.priority error(%s)",
              ec.message().c_str());

  EndPoints endpoints = Resolver::get_endpoints(
    settings->get_i16(port_cfg_name),
    addrs,
    host,
    nets,
    true
  );
  EndPoints endpoints_final;

  for(uint32_t reactor=0; reactor<reactors; ++reactor) {

    auto io_ctx = std::make_shared<asio::io_context>(workers);
    m_wrk.push_back(asio::make_work_guard(*io_ctx.get()));

    for (std::size_t i = 0; i < endpoints.size(); ++i) {
      auto& endpoint = endpoints[i];
      bool ssl_conn = m_ssl_cfg && m_ssl_cfg->need_ssl(endpoint);

      SWC_LOGF(
        LOG_DEBUG, "Binding On: [%s]:%d %s", 
        endpoint.address().to_string().c_str(), endpoint.port(),
        ssl_conn ? "SECURE" : "PLAIN"
      );

      if(reactor == 0) { 
        auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), endpoint);
        if(ssl_conn)
          m_acceptors.push_back(
            std::make_shared<AcceptorSSL>(acceptor, app_ctx, m_ssl_cfg));
        else
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx));
        endpoints_final.push_back(m_acceptors[i]->sock()->local_endpoint());

      } else {
        auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), 
          endpoint.protocol(), dup(m_acceptors[i]->sock()->native_handle()));
        if(ssl_conn)
          m_acceptors.push_back(
            std::make_shared<AcceptorSSL>(acceptor, app_ctx, m_ssl_cfg));
        else
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx));
      }
    }

    if(reactor == 0) {
      if(nets.empty()) {
        app_ctx->init(endpoints_final);
      } else {
        EndPoints sorted;
        Resolver::sort(nets, endpoints_final, sorted);
        app_ctx->init(sorted);
      }
    }

    asio::thread_pool* pool = new asio::thread_pool(workers);
    for(int n=0; n<workers; ++n)
      asio::post(*pool, [this, d=io_ctx] {
        // SWC_LOGF(LOG_INFO, "START DELAY: %s 3secs",  m_appname.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        for(;;) {
          d->run();
          if(m_run.load())
            d->restart();
          else
            break;
        }
      });
    m_thread_pools.push_back(pool);

  }
}

void SerializedServer::run() {
  for(;;) {
    auto it = m_thread_pools.begin();
    if(it == m_thread_pools.end())
      break;
    (*it)->join();
    delete *it;
    m_thread_pools.erase(it);
  }
      
  SWC_LOGF(LOG_INFO, "STOPPED SERVER: %s", m_appname.c_str());
}

void SerializedServer::stop_accepting() {
  for(auto it = m_acceptors.begin(); it < m_acceptors.end(); ) {
    (*it)->stop();
    m_acceptors.erase(it);
  }

  SWC_LOGF(LOG_INFO, "STOPPED ACCEPTING: %s", m_appname.c_str());
}

void SerializedServer::shutdown() {
  SWC_LOGF(LOG_INFO, "STOPPING SERVER: %s", m_appname.c_str());
  m_run.store(false);
  
  ConnHandlerPtr conn;
  for(;;) {
    {
      Mutex::scope lock(m_mutex);
      auto it = m_conns.begin();
      if(it == m_conns.end())
        break;
      conn = *it;
      m_conns.erase(it);
    }
    conn->close();
  }
  
  for (std::size_t i = 0; i < m_wrk.size(); ++i)
    m_wrk[i].reset();
    //m_wrk[i].get_executor().context().stop();
}

void SerializedServer::connection_add(const ConnHandlerPtr& conn) {
  Mutex::scope lock(m_mutex);
  m_conns.push_back(conn);

  //SWC_LOGF(LOG_DEBUG, "%s, conn-add open=%d", m_appname.c_str(), m_conns.size());
}

void SerializedServer::connection_del(const ConnHandlerPtr& conn) {
  Mutex::scope lock(m_mutex);
  for(auto it=m_conns.begin(); it<m_conns.end(); ++it) {
    if(conn->endpoint_remote == (*it)->endpoint_remote){
      m_conns.erase(it);
      break;
    }
  }
  //SWC_LOGF(LOG_DEBUG, "%s, conn-del open=%d", m_appname.c_str(), m_conns.size());
}

SerializedServer::~SerializedServer() {
  if(m_ssl_cfg)
    delete m_ssl_cfg;
}

}}
