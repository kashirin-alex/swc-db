/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/SerializedClient.h"


namespace SWC { namespace Comm { namespace client {



Serialized::Serialized(const Config::Settings& settings,
                       std::string&& srv_name, const IoContextPtr& ioctx,
                       const AppContext::Ptr& ctx)
            : m_srv_name(std::move(srv_name)), m_ioctx(ioctx), m_ctx(ctx),
              m_ssl_cfg(
                settings.get_bool("swc.comm.ssl")
                  ? new ConfigSSL(settings)
                  : nullptr
              ),
              m_run(true), m_calls(0) {
  SWC_LOGF(LOG_DEBUG, "Init: %s", m_srv_name.c_str());
}

Serialized::~Serialized() noexcept {
  if(m_ssl_cfg)
    delete m_ssl_cfg;
}

// Sync
ConnHandlerPtr Serialized::get_connection(
      const EndPoints& endpoints,
      const std::chrono::milliseconds& timeout,
      uint16_t probes) noexcept {
  m_calls.fetch_add(1);
  ConnHandlerPtr conn;
  (void) timeout;
  if(endpoints.empty()) {
    SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints",
                        m_srv_name.c_str());

  } else if(m_run && m_ioctx->running) {
    uint16_t tries = probes;
    do {
      for(auto& endpoint : endpoints) {
        bool need_ssl = m_ssl_cfg && m_ssl_cfg->need_ssl(endpoint);
        SWC_LOG_OUT(LOG_DEBUG, SWC_LOG_OSTREAM << "Connecting Sync: "
          << m_srv_name << ' ' << endpoint << ' '
          << (need_ssl ? "SECURE" : "PLAIN"); );

        asio::error_code ec;
        asio::ip::tcp::socket sock(m_ioctx->executor());
        sock.open(endpoint.protocol(), ec);
        if(!ec && sock.is_open()) {
          sock.connect(endpoint, ec);
          if(!ec && sock.is_open()) try {
            conn = need_ssl && sock.remote_endpoint().address()
                                != sock.local_endpoint().address()
              ? m_ssl_cfg->make_client(m_ctx, sock, ec)
              : ConnHandlerPlain::make(m_ctx, sock);
            if(!ec && conn->is_open()) {
              conn->new_connection();
              goto _return;
            }
          } catch(...) {
            SWC_LOG_CURRENT_EXCEPTION("");
          }
        }
        sock.close(ec);
      }
      SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%u",
                           m_srv_name.c_str(), tries);
      std::this_thread::sleep_for(
        std::chrono::milliseconds(3000)); // ? cfg-setting
    } while (m_run && m_ioctx->running && (!probes || --tries));
  }

  _return:
    m_calls.fetch_sub(1);
    return conn;
}




void Serialized::print(std::ostream& out, ConnHandlerPtr& conn) {
  conn->print(out << m_srv_name << ' ');
}

void Serialized::stop() {
  m_run.store(false);

  for(size_t c = 0; m_calls; ++c) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    if(c % 5000 == 0)
      SWC_LOGF(LOG_WARN,
        "Waiting: %s count(wait=" SWC_FMT_LU " due=" SWC_FMT_LU ")",
        m_srv_name.c_str(), c, m_calls.load());
  }
  SWC_LOGF(LOG_INFO, "Stop: %s", m_srv_name.c_str());
}



}}}
