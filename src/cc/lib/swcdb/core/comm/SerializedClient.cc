/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/SerializedClient.h"


namespace SWC { namespace Comm { namespace client {



ServerConnections::~ServerConnections() noexcept { }

void ServerConnections::reusable(ConnHandlerPtr& conn, bool preserve) {
  while(pop(&(conn = nullptr)) && !conn->is_open());
  if(preserve && conn)
    push(conn);
  /*
  if(conn)
    SWC_LOGF(LOG_DEBUG, "Reusing connection: %s, %s",
             m_srv_name.c_str(), to_string(conn).c_str());
  */
}

void ServerConnections::connection(ConnHandlerPtr& conn,
                                   const std::chrono::milliseconds&,
                                   bool preserve) {

  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_OSTREAM << "Connecting Sync: " << m_srv_name << ' '
      << m_endpoint << ' ' << (m_ssl_cfg ? "SECURE" : "PLAIN"); );

  asio::ip::tcp::socket sock(m_ioctx->executor());
  asio::error_code ec;
  sock.open(m_endpoint.protocol(), ec);
  if(ec || !sock.is_open())
    return;

  sock.connect(m_endpoint, ec);
  if(ec || !sock.is_open())
    return;

  try {
    conn = m_ssl_cfg && sock.remote_endpoint().address()
                         != sock.local_endpoint().address()
      ? m_ssl_cfg->make_client(m_ctx, sock, ec)
      : ConnHandlerPlain::make(m_ctx, sock);
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
    conn = nullptr;
    sock.close(ec);
    return;
  }
  if(!ec && conn->is_open()) {
    conn->new_connection();
    if(preserve)
      push(conn);
    // SWC_LOGF(LOG_DEBUG, "New connection: %s, %s",
    //          m_srv_name.c_str(), to_string(conn).c_str());
  } else {
    conn = nullptr;
  }
}

void ServerConnections::connection(const std::chrono::milliseconds&,
                                   NewCb_t&& cb, bool preserve) {

  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_OSTREAM << "Connecting Async: " << m_srv_name << ' '
      << m_endpoint << ' ' << (m_ssl_cfg ? "SECURE" : "PLAIN"); );

  struct Handshaker {
    ConnHandlerSSL::Ptr     conn;
    NewCb_t                 callback;
    SWC_CAN_INLINE
    Handshaker(const ConnHandlerSSL::Ptr& a_conn, NewCb_t&& a_cb)
      : conn(a_conn), callback(std::move(a_cb)) {
    }
    ~Handshaker() noexcept { }
    void operator()(const asio::error_code& ec) {
      if(ec || !conn->is_open()) {
        conn->do_close();
        conn = nullptr;
      } else {
        conn->new_connection();
      }
      callback(conn);
    }
  };
  struct Handshaker_preserve {
    ServerConnections::Ptr  ptr;
    ConnHandlerSSL::Ptr     conn;
    NewCb_t                 callback;
    SWC_CAN_INLINE
    Handshaker_preserve(ServerConnections::Ptr&& a_ptr,
                        const ConnHandlerSSL::Ptr& a_conn, NewCb_t&& a_cb)
      : ptr(std::move(a_ptr)), conn(a_conn), callback(std::move(a_cb)) {
    }
    ~Handshaker_preserve() noexcept { }
    void operator()(const asio::error_code& ec) {
      if(ec || !conn->is_open()) {
        conn->do_close();
        conn = nullptr;
      } else {
        conn->new_connection();
        if(ptr)
          ptr->push(conn);
      }
      callback(conn);
    }
  };
  struct Connector {
    ServerConnections::Ptr ptr;
    std::shared_ptr<asio::ip::tcp::socket> socket;
    const bool             preserve;
    NewCb_t                callback;
    SWC_CAN_INLINE
    Connector(ServerConnections::Ptr&& a_ptr, bool a_preserve, NewCb_t&& a_cb)
      : ptr(std::move(a_ptr)),
        socket(new asio::ip::tcp::socket(ptr->m_ioctx->executor())),
        preserve(a_preserve), callback(std::move(a_cb)) {
    }
    ~Connector() noexcept { }
    void operator()(const std::error_code& _ec) {
      try {
        if(!_ec && socket->is_open()) {
          if(ptr->m_ssl_cfg && socket->remote_endpoint().address()
                                != socket->local_endpoint().address()) {
            auto conn = ptr->m_ssl_cfg->make_client(
              ptr->m_ctx,  *socket.get());
            preserve
              ? conn->handshake(
                  SocketSSL::client,
                  Handshaker_preserve(
                    std::move(ptr), conn, std::move(callback)))
              : conn->handshake(
                  SocketSSL::client,
                  Handshaker(
                    conn, std::move(callback)));

          } else {
            auto conn = ConnHandlerPlain::make(ptr->m_ctx, *socket.get());
            conn->new_connection();
            if(preserve)
              ptr->push(conn);
            callback(conn);
          }
          return;
        }
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
      }
      std::error_code ec;
      socket->close(ec);
      callback(nullptr);
    }
  };

  Connector hdlr(shared_from_this(), preserve, std::move(cb));
  auto sock = hdlr.socket;
  sock->async_connect(m_endpoint, std::move(hdlr));
}

void ServerConnections::close_all() {
  ConnHandlerPtr conn;
  while(pop(&conn)) conn->do_close();
}


Serialized::Serialized(const Config::Settings& settings,
                       std::string&& srv_name, const IoContextPtr& ioctx,
                       const AppContext::Ptr& ctx)
            : m_srv_name(std::move(srv_name)), m_ioctx(ioctx), m_ctx(ctx),
              m_use_ssl(settings.get_bool("swc.comm.ssl")),
              m_ssl_cfg(m_use_ssl ? new ConfigSSL(settings) : nullptr),
              m_run(true), m_calls(0) {
  SWC_LOGF(LOG_DEBUG, "Init: %s", m_srv_name.c_str());
}

ServerConnections::Ptr Serialized::get_srv(const EndPoint& endpoint) {
  size_t hash = endpoint_hash(endpoint);
  const_iterator it;
  Core::MutexSptd::scope lock(m_mutex);
  if((it = find(hash)) == cend())
    it = emplace(
      hash,
      new ServerConnections(
        m_srv_name, endpoint, m_ioctx, m_ctx,
        m_use_ssl && m_ssl_cfg->need_ssl(endpoint) ? m_ssl_cfg : nullptr
      )
    ).first;

  return it->second;
}

ConnHandlerPtr Serialized::get_connection(
      const EndPoints& endpoints,
      const std::chrono::milliseconds& timeout,
      uint32_t probes, bool preserve) noexcept {
  m_calls.fetch_add(1);
  try {
    return _get_connection(endpoints, timeout, probes, preserve);
  } catch (...) {
    SWC_LOG_CURRENT_EXCEPTION("");
    m_calls.fetch_sub(1);
    return nullptr;
  }
}

ConnHandlerPtr Serialized::_get_connection(
      const EndPoints& endpoints,
      const std::chrono::milliseconds& timeout,
      uint32_t probes, bool preserve) {

  ConnHandlerPtr conn = nullptr;
  if(endpoints.empty()) {
    SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints",
                        m_srv_name.c_str());
    m_calls.fetch_sub(1);
    return conn;
  }

  ServerConnections::Ptr srv;
  uint32_t tries = probes;
  if(m_run && m_ioctx->running) do {

    for(auto& endpoint : endpoints){
      srv = get_srv(endpoint);
      srv->reusable(conn, preserve);
      if(!conn)
        srv->connection(conn, timeout, preserve);
      if(conn) {
        m_calls.fetch_sub(1);
        return conn;
      }
    }
    SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%u",
                         m_srv_name.c_str(), tries);

    std::this_thread::sleep_for(
      std::chrono::milliseconds(3000)); // ? cfg-setting

  } while (m_run && m_ioctx->running && (!probes || --tries));

  m_calls.fetch_sub(1);
  return conn;
}

void Serialized::get_connection(
      const EndPoints& endpoints,
      ServerConnections::NewCb_t&& cb,
      const std::chrono::milliseconds& timeout,
      uint32_t probes, bool preserve) noexcept {
  m_calls.fetch_add(1);
  if(m_run) try {
    if(endpoints.empty()) {
      SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints",
                          m_srv_name.c_str());
    } else {
      _get_connection(
        endpoints, std::move(cb), timeout, probes, probes, 0, preserve);
      return;
    }
  } catch (...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  cb(nullptr);
  m_calls.fetch_sub(1);
}

void Serialized::_get_connection(
      const EndPoints& endpoints,
      ServerConnections::NewCb_t&& cb,
      const std::chrono::milliseconds& timeout,
      uint32_t probes, uint32_t tries,
      size_t next, bool preserve) {

  if(!m_run || !m_ioctx->running || endpoints.empty()) {
    cb(nullptr);
    m_calls.fetch_sub(1);
    return;
  }

  if(next >= endpoints.size())
    next = 0;

  auto srv = get_srv(endpoints[next]);
  ConnHandlerPtr _conn = nullptr;
  srv->reusable(_conn, preserve);
  if(!m_run || _conn || (probes && !tries)) {
    cb(_conn);
    m_calls.fetch_sub(1);
    return;
  }

  ++next;
  SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%u",
           m_srv_name.c_str(), tries);
  srv->connection(timeout,
    [endpoints, timeout, probes, tries, next, preserve,
     cb=std::move(cb), ptr=shared_from_this()]
    (const ConnHandlerPtr& conn) mutable {
      if(!ptr->m_run.load() || (conn && conn->is_open())) {
        cb(conn);
        ptr->m_calls.fetch_sub(1);
        return;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // ? cfg-setting
      try {
        ptr->_get_connection(
          endpoints, std::move(cb), timeout,
          probes, next == endpoints.size() ? tries-1 : tries,
          next,
          preserve
        );
      } catch (...) {
        SWC_LOG_CURRENT_EXCEPTION("");
        cb(nullptr);
        ptr->m_calls.fetch_sub(1);
      }
    },
    preserve
  );
}

void Serialized::preserve(ConnHandlerPtr& conn) {
  size_t hash = conn->endpoint_remote_hash();
  const_iterator it;
  Core::MutexSptd::scope lock(m_mutex);
  if((it = find(hash)) != cend())
    it->second->push(conn);
}

/*
void Serialized::close(ConnHandlerPtr& conn) {
  size_t hash = conn->endpoint_remote_hash();
  conn->do_close();
  const_iterator it;
  Core::MutexSptd::scope lock(m_mutex);
  if((it = find(hash)) != cend() && it->second->empty())
    erase(it);
}
*/

void Serialized::print(std::ostream& out, ConnHandlerPtr& conn) {
  conn->print(out << m_srv_name << ' ');
}

void Serialized::stop() {
  m_run.store(false);

  const_iterator it;
  for(ServerConnections::Ptr srv;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if((it = cbegin()) == cend())
        break;
      srv = std::move(it->second);
      erase(it);
    }
    srv->close_all();
    for(size_t c = 0; m_calls || srv.use_count() > 1; ++c) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      if(c % 5000 == 0)
        SWC_LOGF(LOG_WARN, "Waiting: %s count(wait=%lu use=%ld due=%lu)",
                  m_srv_name.c_str(), c, srv.use_count(), m_calls.load());
    }
  }
  SWC_LOGF(LOG_INFO, "Stop: %s", m_srv_name.c_str());
}

Serialized::~Serialized() noexcept {
  if(m_ssl_cfg)
    delete m_ssl_cfg;
}



}}}
