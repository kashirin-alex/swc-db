/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_SerializedClient_h
#define swcdb_core_comm_SerializedClient_h


#include "swcdb/core/QueueSafe.h"
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/comm/ConfigSSL.h"


namespace SWC { namespace Comm {


//! The SWC-DB client C++ namespace 'SWC::Comm::client'
namespace client {



class Serialized final : public std::enable_shared_from_this<Serialized> {
  public:

  typedef std::shared_ptr<Serialized> Ptr;

  Serialized(const Config::Settings& settings,
             std::string&& srv_name,
             const IoContextPtr& ioctx,
             const AppContext::Ptr& ctx);

  Serialized(Serialized&&)                 = delete;
  Serialized(const Serialized&)            = delete;
  Serialized& operator=(Serialized&&)      = delete;
  Serialized& operator=(const Serialized&) = delete;

  virtual ~Serialized() noexcept;

  SWC_CAN_INLINE
  IoContextPtr io() noexcept {
    return m_ioctx;
  }

  ConnHandlerPtr get_connection(
        const EndPoints& endpoints,
        const std::chrono::milliseconds& timeout,
        uint16_t probes
  ) noexcept;


  template<typename HdlrT>
  SWC_CAN_INLINE
  void get_connection(
        const EndPoints& endpoints,
        HdlrT&& cb,
        const std::chrono::milliseconds& timeout,
        uint16_t probes) noexcept {
    get_connection<HdlrT>(endpoints, timeout, probes, std::move(cb));
  }

  template<typename HdlrT, typename... DataArgsT>
  SWC_CAN_INLINE
  void get_connection(
        const EndPoints& endpoints,
        const std::chrono::milliseconds& timeout,
        uint16_t probes,
        DataArgsT&&... args) noexcept {
    m_calls.fetch_add(1);
    if(m_run) try {
      if(endpoints.empty()) {
        SWC_LOGF(LOG_WARN, "get_connection: %s, Empty-Endpoints",
                            m_srv_name.c_str());
      } else {
        (void)timeout;
        return Connector<HdlrT>(
          shared_from_this(),
          endpoints,
          probes,
          std::forward<DataArgsT>(args)...
        ).connect();
      }
    } catch (...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
    HdlrT(std::forward<DataArgsT>(args)...)(nullptr);
    m_calls.fetch_sub(1);
  }

  void print(std::ostream& out, ConnHandlerPtr& conn);

  void stop();

  private:

  template<typename HdlrT>
  struct Connector {
    using SocketPtr = std::shared_ptr<asio::ip::tcp::socket>;

    Serialized::Ptr           ptr;
    EndPoints                 endpoints;
    //std::chrono::milliseconds timeout;
    uint16_t                  probes;
    uint16_t                  tries;
    uint32_t                  next;
    SocketPtr                 socket;
    HdlrT                     callback;

    template<typename... DataArgsT>
    SWC_CAN_INLINE
    Connector(Serialized::Ptr&& a_ptr,
              const EndPoints& a_endpoints, uint16_t a_probes,
              DataArgsT&&... args)
        : ptr(std::move(a_ptr)),
          endpoints(a_endpoints), probes(a_probes), tries(a_probes), next(0),
          socket(nullptr), callback(std::forward<DataArgsT>(args)...) {
    }

    void connect() {
      if(next >= endpoints.size())
        next = 0;
      auto endpoint = endpoints[next];
      SWC_LOG_OUT(LOG_DEBUG, SWC_LOG_OSTREAM << "Connecting Async: "
        << ptr->m_srv_name << ' ' << endpoint << ' '
        << (ptr->m_ssl_cfg && ptr->m_ssl_cfg->need_ssl(endpoint)
              ? "SECURE" : "PLAIN"); );

      socket.reset(new asio::ip::tcp::socket(ptr->m_ioctx->executor()));
      auto sock = socket;
      sock->async_connect(endpoint, std::move(*this));
    }

    ~Connector() noexcept { }

    void operator()(const std::error_code& _ec) noexcept {
      if(_ec) {
        asio::error_code ec;
        socket->close(ec);
      } else if(socket->is_open()) try {

        if(ptr->m_ssl_cfg &&
           ptr->m_ssl_cfg->need_ssl(endpoints[next]) &&
           socket->remote_endpoint().address()
            != socket->local_endpoint().address()) {
          struct Handshaker {
            ConnHandlerSSL::Ptr conn;
            Connector<HdlrT>    connector;
            SWC_CAN_INLINE
            Handshaker(const ConnHandlerSSL::Ptr& a_conn,
                       Connector<HdlrT>&& hdlr)
              : conn(a_conn), connector(std::move(hdlr)) {
            }
            ~Handshaker() noexcept { }
            void operator()(const asio::error_code& ec) {
              if(conn->is_open()) {
                if(!ec) {
                  conn->new_connection();
                  connector.callback(conn);
                  connector.ptr->m_calls.fetch_sub(1);
                  return;
                }
                conn->do_close();
              }
              connector.reconnect();
            }
          };
          auto conn = ptr->m_ssl_cfg->make_client(ptr->m_ctx,  *socket.get());
          conn->handshake(
            SocketSSL::client, Handshaker(conn, std::move(*this)));
          return;
        }

        auto conn = ConnHandlerPlain::make(ptr->m_ctx, *socket.get());
        if(conn->is_open()) {
          conn->new_connection();
          callback(conn);
          ptr->m_calls.fetch_sub(1);
          return;
        }
      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
      }

      reconnect();
    }

    void reconnect() noexcept {
      if(ptr->m_run && ptr->m_ioctx->running && !endpoints.empty() &&
         (!probes || tries)) try {
        SWC_LOGF(LOG_DEBUG, "get_connection: %s, tries=%u",
                 ptr->m_srv_name.c_str(), tries);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // ? cfg-setting

        if(++next == endpoints.size())
          --tries;
        return connect();

      } catch(...) {
        SWC_LOG_CURRENT_EXCEPTION("");
      }
      callback(nullptr);
      ptr->m_calls.fetch_sub(1);
    }

  };

  const std::string     m_srv_name;
  IoContextPtr          m_ioctx;
  AppContext::Ptr       m_ctx;
  ConfigSSL*            m_ssl_cfg;
  Core::AtomicBool      m_run;
  Core::Atomic<size_t>  m_calls;

};


}}} // namespace SWC::Comm::client


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedClient.cc"
#endif

#endif // swcdb_core_comm_SerializedClient_h
