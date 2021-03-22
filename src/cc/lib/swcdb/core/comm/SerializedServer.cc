/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include <unordered_map>


namespace SWC { namespace Comm { namespace server {

Acceptor::Acceptor(asio::ip::tcp::acceptor& acceptor,
                   AppContext::Ptr& app_ctx,
                   ConfigSSL* ssl_cfg)
                  : asio::ip::tcp::acceptor(std::move(acceptor)),
                    m_app_ctx(app_ctx),
                    m_ssl_cfg(ssl_cfg) {
  set_option(asio::ip::tcp::acceptor::reuse_address(true));
  set_option(asio::ip::tcp::no_delay(true));

  m_ssl_cfg ? do_accept_mixed() : do_accept_plain();

  SWC_LOG_OUT(LOG_INFO,
    SWC_LOG_OSTREAM << "Listening On: " << local_endpoint()
      << " fd=" << native_handle()
      << ' ' << (m_ssl_cfg ? "SECURE" : "PLAIN");
  );
}

void Acceptor::stop() {
  SWC_LOG_OUT(LOG_INFO,
    SWC_LOG_OSTREAM << "Stopping to Listen On: " << local_endpoint()
      << " fd=" << native_handle();
  );

  if(is_open()) {
    std::error_code ec;
    close(ec);
  }
}

Acceptor::~Acceptor() { }

asio::ip::tcp::acceptor* Acceptor::sock() noexcept {
  return this;
}

void Acceptor::do_accept_mixed() noexcept {
  async_accept(
    [this](std::error_code ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != ECANCELED)
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)",
                    ec.value(), ec.message().c_str());
        return;
      }

      if(new_sock.is_open()) try {
        // OR m_ssl_cfg->need_ssl(new_sock.remote_endpoint(ec))
        bool need_ssl = new_sock.remote_endpoint(ec).address() !=
                        new_sock.local_endpoint(ec).address();
        if(!ec) {
          if(need_ssl) {
            m_ssl_cfg->make_server(m_app_ctx, new_sock);
          } else {
            auto conn = std::make_shared<ConnHandlerPlain>(m_app_ctx, new_sock);
            conn->new_connection();
          }
        } else if(new_sock.is_open()) {
          new_sock.close(ec);
        }
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
        new_sock.close(ec);
      }

      do_accept_mixed();
    }
  );
}

void Acceptor::do_accept_plain() noexcept {
  async_accept(
    [this](const std::error_code& ec, asio::ip::tcp::socket new_sock) {
      if(ec) {
        if(ec.value() != ECANCELED)
          SWC_LOGF(LOG_DEBUG, "SRV-accept error=%d(%s)",
                    ec.value(), ec.message().c_str());
        return;
      }

      if(new_sock.is_open()) try {
        auto conn = std::make_shared<ConnHandlerPlain>(m_app_ctx, new_sock);
        conn->new_connection();
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
        std::error_code ec;
        new_sock.close(ec);
      }

      do_accept_plain();
    }
  );
}



SerializedServer::SerializedServer(
    const std::string& name,
    uint32_t reactors, uint32_t workers,
    const std::string& port_cfg_name,
    AppContext::Ptr app_ctx
  ) : m_appname(name), m_run(true),
      m_ssl_cfg(Env::Config::settings()->get_bool("swc.comm.ssl")
                ? new ConfigSSL(false) : nullptr) {

  SWC_LOGF(LOG_INFO, "STARTING SERVER: %s, reactors=%d, workers=%d",
           m_appname.c_str(), reactors, workers);

  auto settings = Env::Config::settings();

  Config::Strings addrs;
  if(settings->has("addr"))
    addrs = settings->get_strs("addr");

  std::string host;
  if(settings->has("host"))
    host = host.append(settings->get_str("host"));
  else {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }

  Networks nets;
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
    m_io_contexts.push_back(
      IoContext::make(name + "-reactor-" + std::to_string(reactor+1),
      workers
    ));

    for (std::size_t i = 0; i < endpoints.size(); ++i) {
      auto& endpoint = endpoints[i];
      bool ssl_conn = m_ssl_cfg && m_ssl_cfg->need_ssl(endpoint);

      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_OSTREAM << "Binding On: " << endpoint
          << ' ' << (ssl_conn ? "SECURE" : "PLAIN");
      );

      if(!reactor) {
        auto acceptor = asio::ip::tcp::acceptor(
          m_io_contexts.back()->executor(),
          endpoint
        );
        m_acceptors.push_back(std::make_shared<Acceptor>(
          acceptor, app_ctx, ssl_conn ? m_ssl_cfg : nullptr));
        endpoints_final.push_back(m_acceptors[i]->sock()->local_endpoint());

      } else {
        auto acceptor = asio::ip::tcp::acceptor(
          m_io_contexts.back()->executor(),
          endpoint.protocol(), dup(m_acceptors[i]->sock()->native_handle())
        );
        m_acceptors.push_back(std::make_shared<Acceptor>(
          acceptor, app_ctx, ssl_conn ? m_ssl_cfg : nullptr));
      }
    }

    if(!reactor) {
      if(nets.empty()) {
        app_ctx->init(endpoints_final);
      } else {
        EndPoints sorted;
        Resolver::sort(nets, endpoints_final, sorted);
        app_ctx->init(sorted);
      }
    }
  }
}

void SerializedServer::run() {
  for(auto& io : m_io_contexts)
    io->pool.join();

  SWC_LOGF(LOG_INFO, "STOPPED SERVER: %s", m_appname.c_str());
}

std::shared_ptr<IoContext::ExecutorWorkGuard>
SerializedServer::stop_accepting() {
  auto guard = std::make_shared<IoContext::ExecutorWorkGuard>(
    m_io_contexts.back()->executor());

  for(auto it = m_acceptors.begin(); it != m_acceptors.end(); ) {
    (*it)->stop();
    m_acceptors.erase(it);
  }

  SWC_LOGF(LOG_INFO, "STOPPED ACCEPTING: %s", m_appname.c_str());
  return guard;
}

void SerializedServer::shutdown() {
  SWC_LOGF(LOG_INFO, "STOPPING SERVER: %s", m_appname.c_str());
  m_run.store(false);

  ConnHandlerPtr conn;
  for(;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = m_conns.begin();
      if(it == m_conns.end())
        break;
      conn = *it;
      m_conns.erase(it);
    }
    conn->do_close();
  }
  //for(auto& io : m_io_contexts)
  //  io->stop();
}

void SerializedServer::connection_add(const ConnHandlerPtr& conn) {
  Core::MutexSptd::scope lock(m_mutex);
  m_conns.push_back(conn);

  //SWC_LOGF(LOG_DEBUG, "%s, conn-add open=%d", m_appname.c_str(), m_conns.size());
}

void SerializedServer::connection_del(const ConnHandlerPtr& conn) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it=m_conns.begin(); it != m_conns.end(); ++it) {
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



}}}
