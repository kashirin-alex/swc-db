/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"


namespace SWC { namespace Comm { namespace server {



struct Acceptor::Mixed {

  struct Handshaker {
    ConnHandlerSSL::Ptr conn;
    SWC_CAN_INLINE
    Handshaker(const ConnHandlerSSL::Ptr& a_conn) : conn(a_conn) { }
    SWC_CAN_INLINE
    Handshaker(Handshaker&& other) noexcept : conn(std::move(other.conn)) { }
    Handshaker(const Handshaker&) = delete;
    Handshaker& operator=(Handshaker&&) = delete;
    Handshaker& operator=(const Handshaker&) = delete;
    ~Handshaker() noexcept { }
    void operator()(const asio::error_code& ec) {
      if(!ec) {
        conn->new_connection();
      } else {
        SWC_LOGF(LOG_DEBUG, "handshake error=%d(%s)",
                  ec.value(), ec.message().c_str());
      }
    }
  };

  Acceptor* acceptor;
  SWC_CAN_INLINE
  Mixed(Acceptor* a_acceptor) noexcept : acceptor(a_acceptor) { }
  void operator()(std::error_code ec, asio::ip::tcp::socket new_sock) {
    bool need_ssl = false;
    if(ec) {
      if(ec.value() == ECANCELED)
        return;
      SWC_LOGF(LOG_WARN, "SRV-accept error=%d(%s)",
               ec.value(), ec.message().c_str());

    } else if(new_sock.is_open()) {
      try {
        need_ssl = acceptor->m_ssl_cfg->need_ssl(
          new_sock.local_endpoint(ec), new_sock.remote_endpoint(ec));
        if(!ec) {
          if(need_ssl) {
            auto conn = acceptor->m_ssl_cfg->make_connection(
              acceptor->m_app_ctx, new_sock);
            conn->handshake(SocketSSL::server, Handshaker(conn));
          } else {
            auto conn = ConnHandlerPlain::make(
              acceptor->m_app_ctx, new_sock);
            conn->new_connection();
          }
        } else if(new_sock.is_open()) {
          new_sock.close(ec);
        }
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
        new_sock.close(ec);
      }
    }

    acceptor->m_app_ctx->net_accepted(acceptor->local_endpoint(), need_ssl);
    acceptor->async_accept(Mixed(acceptor));
  }
};

struct Acceptor::Plain {
  Acceptor* acceptor;
  SWC_CAN_INLINE
  Plain(Acceptor* a_acceptor) noexcept : acceptor(a_acceptor) { }
  void operator()(const std::error_code& ec, asio::ip::tcp::socket new_sock) {
    if(ec) {
      if(ec.value() == ECANCELED)
        return;
      SWC_LOGF(LOG_WARN, "SRV-accept error=%d(%s)",
               ec.value(), ec.message().c_str());

    } else if(new_sock.is_open()) {
      try {
        auto conn = ConnHandlerPlain::make(acceptor->m_app_ctx, new_sock);
        conn->new_connection();
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
        std::error_code _ec;
        new_sock.close(_ec);
      }
    }

    acceptor->m_app_ctx->net_accepted(acceptor->local_endpoint(), false);
    acceptor->async_accept(Plain(acceptor));
  }
};

SWC_SHOULD_NOT_INLINE
Acceptor::Acceptor(asio::ip::tcp::acceptor& acceptor,
                   AppContext::Ptr& app_ctx,
                   ConfigSSL* ssl_cfg)
                  : asio::ip::tcp::acceptor(std::move(acceptor)),
                    m_app_ctx(app_ctx),
                    m_ssl_cfg(ssl_cfg) {
  set_option(asio::ip::tcp::acceptor::reuse_address(true));
  set_option(asio::ip::tcp::no_delay(true));

  if(m_ssl_cfg)
    async_accept(Mixed(this));
  else
    async_accept(Plain(this));

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


SWC_SHOULD_NOT_INLINE
SerializedServer::SerializedServer(
    const Config::Settings& settings,
    std::string&& name,
    uint32_t reactors, uint32_t workers,
    uint16_t port,
    AppContext::Ptr app_ctx
  ) : m_appname(std::move(name)), m_run(true),
      m_io_contexts(), m_acceptors(), m_mutex(), m_conns(),
      m_ssl_cfg(
        settings.get_bool("swc.comm.ssl")
          ? new ConfigSSL(settings, false)
          : nullptr
      ) {

  SWC_LOGF(LOG_INFO, "STARTING SERVER: %s, reactors=%u, workers=%u",
           m_appname.c_str(), reactors, workers);

  Config::Strings addrs;
  if(settings.has("addr"))
    addrs = settings.get_strs("addr");

  std::string host;
  if(settings.has("host"))
    host = host.append(settings.get_str("host"));
  else {
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) == -1)
      SWC_THROW(errno, "gethostname");
    host.append(hostname);
  }

  Networks nets;
  asio::error_code ec;
  Resolver::get_networks(
    settings.get_strs("swc.comm.network.priority"), nets, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "swc.comm.network.priority error(%s)",
              ec.message().c_str());

  EndPoints endpoints = Resolver::get_endpoints(
    port,
    addrs,
    host,
    nets,
    true
  );
  EndPoints endpoints_final;

  for(uint32_t reactor=0; reactor<reactors; ++reactor) {
    m_io_contexts.push_back(
      IoContext::make(m_appname + "-reactor-" + std::to_string(reactor+1),
      workers
    ));

    for (std::size_t i = 0; i < endpoints.size(); ++i) {
      bool ssl_conn = m_ssl_cfg && m_ssl_cfg->need_ssl(endpoints[i]);

      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_OSTREAM << "Binding On: " << endpoints[i]
          << ' ' << (ssl_conn ? "SECURE" : "PLAIN");
      );

      if(!reactor) {
        auto acceptor = asio::ip::tcp::acceptor(
          m_io_contexts.back()->executor(),
          endpoints[i]
        );
        m_acceptors.emplace_back(new Acceptor(
          acceptor, app_ctx, ssl_conn ? m_ssl_cfg : nullptr));
        endpoints_final.push_back(m_acceptors[i]->sock()->local_endpoint());

      } else {
        auto acceptor = asio::ip::tcp::acceptor(
          m_io_contexts.back()->executor(),
          endpoints[i].protocol(),
          dup(m_acceptors[i]->sock()->native_handle())
        );
        m_acceptors.emplace_back(new Acceptor(
          acceptor, app_ctx, ssl_conn ? m_ssl_cfg : nullptr));
      }
    }

    if(!reactor) {
      if(nets.empty()) {
        app_ctx->init(host, endpoints_final);
      } else {
        EndPoints sorted;
        Resolver::sort(nets, endpoints_final, sorted);
        app_ctx->init(host, sorted);
      }
    }
  }
}

SWC_SHOULD_NOT_INLINE
void SerializedServer::run() {
  for(auto& io : m_io_contexts)
    io->pool.join();

  SWC_LOGF(LOG_INFO, "STOPPED SERVER: %s", m_appname.c_str());
}

std::shared_ptr<IoContext::ExecutorWorkGuard>
SerializedServer::stop_accepting() {
  auto guard = std::shared_ptr<IoContext::ExecutorWorkGuard>(
    new IoContext::ExecutorWorkGuard(m_io_contexts.back()->executor()));

  for(auto it = m_acceptors.cbegin(); it != m_acceptors.cend(); ) {
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
      auto it = m_conns.cbegin();
      if(it == m_conns.cend())
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
  for(auto it=m_conns.cbegin(); it != m_conns.cend(); ++it) {
    if(conn->endpoint_remote == (*it)->endpoint_remote){
      m_conns.erase(it);
      break;
    }
  }
  //SWC_LOGF(LOG_DEBUG, "%s, conn-del open=%d", m_appname.c_str(), m_conns.size());
}

SerializedServer::~SerializedServer() noexcept {
  delete m_ssl_cfg;
}



}}}
