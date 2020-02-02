/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <unordered_map>

#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"


namespace SWC { namespace server {

Acceptor::Acceptor(asio::ip::tcp::acceptor& acceptor, 
                  AppContext::Ptr app_ctx, IOCtxPtr io_ctx,
                  bool is_plain)
                  : m_acceptor(std::move(acceptor)), 
                    m_app_ctx(app_ctx), m_io_ctx(io_ctx) {
  SWC_LOGF(
    LOG_INFO, "Listening On: [%s]:%d fd=%d %s", 
    m_acceptor.local_endpoint().address().to_string().c_str(), 
    m_acceptor.local_endpoint().port(), 
    (ssize_t)m_acceptor.native_handle(),
    is_plain ? "PLAIN" : "SECURE"
  );
  if(is_plain)
    do_accept();
}

void Acceptor::stop() {
  SWC_LOGF(LOG_INFO, "Stopping to Listen On: [%s]:%d fd=%d", 
           m_acceptor.local_endpoint().address().to_string().c_str(), 
           m_acceptor.local_endpoint().port(), 
           (ssize_t)m_acceptor.native_handle());

  if(m_acceptor.is_open())
    m_acceptor.close();
}

Acceptor::~Acceptor() { }

asio::ip::tcp::acceptor* Acceptor::sock() {
  return &m_acceptor;
}

void Acceptor::do_accept() {
  m_acceptor.async_accept(
    [this](std::error_code ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != 125) 
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)", 
                    ec.value(), ec.message().c_str());
        return;
      }
      
      auto conn = std::make_shared<ConnHandlerPlain>(m_app_ctx, new_sock);
      conn->new_connection();
      conn->accept_requests();

      do_accept();
    }
  );
}




AcceptorSSL::AcceptorSSL(asio::ip::tcp::acceptor& acceptor, 
                        AppContext::Ptr app_ctx, IOCtxPtr io_ctx,
                        const std::string& ssl_pem,
                        const std::string& ssl_ciphers)
                        : Acceptor(acceptor, app_ctx, io_ctx, false),
                          ssl_pem(ssl_pem), ssl_ciphers(ssl_ciphers) {
  do_accept();
}

void AcceptorSSL::do_accept() {
  m_acceptor.async_accept(
    [this](std::error_code ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != 125) 
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)", 
                    ec.value(), ec.message().c_str());
        return;
      }

      asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_server);
      ssl_ctx.set_options(
          asio::ssl::context::default_workarounds
        | asio::ssl::context::no_compression
        | asio::ssl::context::no_sslv2
        | asio::ssl::context::no_sslv3
        | asio::ssl::context::no_tlsv1
        | asio::ssl::context::no_tlsv1_1
        | SSL_OP_NO_TICKET
        | SSL_OP_EPHEMERAL_RSA
        | SSL_OP_SINGLE_ECDH_USE
        //| asio::ssl::context::no_tlsv1_2
        | asio::ssl::context::single_dh_use
      );
      ssl_ctx.use_certificate_chain_file(ssl_pem);
      ssl_ctx.use_private_key_file(ssl_pem, asio::ssl::context::pem);
      //ssl_ctx.use_tmp_dh_file("dh2048.pem");
      //ssl_ctx.set_password_callback(..);
  
      if(!ssl_ciphers.empty())
        SSL_CTX_set_cipher_list(ssl_ctx.native_handle(), ssl_ciphers.c_str());
      
      auto conn = std::make_shared<ConnHandlerSSL>(m_app_ctx, ssl_ctx, new_sock);
      conn->new_connection();
      conn->handshake();

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
  ): m_appname(name), m_run(true) {
    
  SWC_LOGF(LOG_INFO, "STARTING SERVER: %s, reactors=%d, workers=%d", 
           m_appname.c_str(), reactors, workers);

  auto& props = Env::Config::settings()->properties;

  Strings addrs = props.has("addr") ? props.get<Strings>("addr") : Strings();
  std::string host;
  if(props.has("host"))
    host = host.append(props.get<std::string>("host"));
  else {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }
    
  EndPoints endpoints = Resolver::get_endpoints(
    props.get<int16_t>(port_cfg_name),
    addrs,
    host,
    true
  );

  bool use_ssl = props.get<bool>("swc.comm.ssl");
  std::vector<asio::ip::network_v4> nets_v4;
  std::vector<asio::ip::network_v6> nets_v6;

  std::string ssl_pem = props.get<std::string>("swc.comm.ssl.pem");
  if(ssl_pem.front() != '.' && ssl_pem.front() != '/')
    ssl_pem = props.get<std::string>("swc.cfg.path") + ssl_pem;
  std::string ssl_ciphers = props.has("swc.comm.ssl.ciphers") 
        ? props.get<std::string>("swc.comm.ssl.ciphers") : "";

  if(use_ssl) {
    asio::error_code ec;
    Resolver::get_networks(
      props.get<Strings>("swc.comm.ssl.secure.network"),
      nets_v4,
      nets_v6,
      ec
    );
    if(ec)
      SWC_THROWF(Error::CONFIG_BAD_VALUE,
                "Bad Network in swc.comm.ssl.secure.network error(%s)",
                 ec.message().c_str());
  }
  
  EndPoints endpoints_final;

  for(uint32_t reactor=0; reactor<reactors; ++reactor) {

    auto io_ctx = std::make_shared<asio::io_context>(workers);
    m_wrk.push_back(asio::make_work_guard(*io_ctx.get()));

    for (std::size_t i = 0; i < endpoints.size(); ++i) {
      auto& endpoint = endpoints[i];
      bool ssl_conn = use_ssl && 
                      !Resolver::is_network(endpoint, nets_v4, nets_v6);

      if(reactor == 0) { 
        auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), endpoint);
        if(ssl_conn)
          m_acceptors.push_back(
            std::make_shared<AcceptorSSL>(
              acceptor, app_ctx, io_ctx, ssl_pem, ssl_ciphers));
        else
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx, io_ctx));
        endpoints_final.push_back(m_acceptors[i]->sock()->local_endpoint());

      } else {
        auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), 
          endpoint.protocol(), dup(m_acceptors[i]->sock()->native_handle()));
        if(ssl_conn)
          m_acceptors.push_back(
            std::make_shared<AcceptorSSL>(
              acceptor, app_ctx, io_ctx, ssl_pem, ssl_ciphers));
        else
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx, io_ctx));
      }
    }

    if(reactor == 0)
      app_ctx->init(endpoints_final);

    asio::thread_pool* pool = new asio::thread_pool(workers);
    for(int n=0; n<workers; ++n)
      asio::post(*pool, [d=io_ctx, run=&m_run]{
        // SWC_LOGF(LOG_INFO, "START DELAY: %s 3secs",  m_appname.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        do{
          d->run();
          d->restart();
        }while(run->load());
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
  for(;;) {
    auto it = m_acceptors.begin();
    if(it == m_acceptors.end())
      break;
    (*it)->stop();
    m_acceptors.erase(it);
  }

  SWC_LOGF(LOG_INFO, "STOPPED ACCEPTING: %s", m_appname.c_str());
}

void SerializedServer::shutdown() {
  SWC_LOGF(LOG_INFO, "STOPPING SERVER: %s", m_appname.c_str());
  m_run.store(false);

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& conn : m_conns)
      conn->close();
  }
  for (std::size_t i = 0; i < m_wrk.size(); ++i)
    m_wrk[i].reset();
    //m_wrk[i].get_executor().context().stop();
}

void SerializedServer::connection_add(ConnHandlerPtr conn) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_conns.push_back(conn);

  //SWC_LOGF(LOG_DEBUG, "%s, conn-add open=%d", m_appname.c_str(), m_conns.size());
}

void SerializedServer::connection_del(ConnHandlerPtr conn) {
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto it=m_conns.begin(); it<m_conns.end(); ++it) {
    if(conn->endpoint_remote == (*it)->endpoint_remote){
      m_conns.erase(it);
      break;
    }
  }
  //SWC_LOGF(LOG_DEBUG, "%s, conn-del open=%d", m_appname.c_str(), m_conns.size());
}

SerializedServer::~SerializedServer() {}

}}
