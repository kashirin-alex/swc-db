/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_ConnHandler_h
#define swcdb_core_comm_ConnHandler_h


#include "swcdb/core/Exception.h"
#include "swcdb/core/StateRunning.h"
#include "swcdb/core/QueueSafeStated.h"
#include "swcdb/core/comm/asio_wrap.h"
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/Buffers.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/DispatchHandler.h"


namespace SWC { namespace Comm {

using SocketLayer = asio::ip::tcp::socket::lowest_layer_type;
using SocketPlain = asio::ip::tcp::socket;
using SocketSSL = asio::ssl::stream<asio::ip::tcp::socket>;


class ConnHandler : public std::enable_shared_from_this<ConnHandler> {


  struct Outgoing final {
    Buffers::Ptr                  cbuf;
    DispatchHandler::Ptr          hdlr;

    SWC_CAN_INLINE
    Outgoing() noexcept : cbuf(nullptr), hdlr(nullptr) { }

    SWC_CAN_INLINE
    Outgoing(Buffers::Ptr&& a_cbuf, DispatchHandler::Ptr&& a_hdlr) noexcept
            : cbuf(std::move(a_cbuf)), hdlr(std::move(a_hdlr)) {
    }

    SWC_CAN_INLINE
    Outgoing(Outgoing&& other) noexcept
           : cbuf(std::move(other.cbuf)), hdlr(std::move(other.hdlr)) {
    }

    SWC_CAN_INLINE
    ~Outgoing() noexcept { }

    SWC_CAN_INLINE
    Outgoing& operator=(Outgoing&& other) noexcept {
      cbuf = std::move(other.cbuf);
      hdlr = std::move(other.hdlr);
      return *this;
    }

    Outgoing(const Outgoing&)             = delete;

    Outgoing& operator=(const Outgoing&)  = delete;
  };

  struct Pending final {
    DispatchHandler::Ptr          hdlr;
    asio::high_resolution_timer*  timer;

    SWC_CAN_INLINE
    Pending() noexcept : hdlr(nullptr), timer(nullptr) { }

    SWC_CAN_INLINE
    Pending(DispatchHandler::Ptr&& a_hdlr) noexcept
           : hdlr(std::move(a_hdlr)), timer(nullptr) {
    }

    SWC_CAN_INLINE
    Pending(Pending&& other) noexcept
           : hdlr(std::move(other.hdlr)), timer(other.timer) {
      other.timer = nullptr;
    }

    SWC_CAN_INLINE
    ~Pending() noexcept {
      delete timer;
    }

    SWC_CAN_INLINE
    Pending& operator=(Pending&& other) noexcept {
      hdlr = std::move(other.hdlr);
      timer = other.timer;
      other.timer = nullptr;
      return *this;
    }

    Pending(const Pending&)             = delete;

    Pending& operator=(const Pending&)  = delete;

  };


  public:

  ConnHandler(const ConnHandler&)             = delete;
  ConnHandler(ConnHandler&&)                  = delete;
  ConnHandler& operator=(const ConnHandler&)  = delete;
  ConnHandler& operator=(ConnHandler&&)       = delete;

  Core::AtomicBool      connected;
  const AppContext::Ptr app_ctx;
  EndPoint              endpoint_remote;
  EndPoint              endpoint_local;

  SWC_CAN_INLINE
  size_t endpoint_remote_hash() const noexcept {
    return endpoint_hash(endpoint_remote);
  }

  SWC_CAN_INLINE
  size_t endpoint_local_hash() const noexcept {
    return endpoint_hash(endpoint_local);
  }

  SWC_CAN_INLINE
  Core::Encoder::Type get_encoder() const noexcept {
    return !app_ctx->cfg_encoder ||
            endpoint_local.address() == endpoint_remote.address()
              ? Core::Encoder::Type::PLAIN
              : Core::Encoder::Type(app_ctx->cfg_encoder->get());
  }

  virtual bool is_secure() const noexcept { return false; };

  void new_connection();

  virtual bool is_open() const noexcept = 0;

  size_t pending_read() noexcept;

  SWC_CAN_INLINE
  bool due() {
    return m_outgoing.is_active() || m_outgoing.size() || pending_read();
  }

  virtual void do_close() noexcept = 0;

  bool send_error(int error, const std::string& msg,
                  const Event::Ptr& ev) noexcept;

  bool response_ok(const Event::Ptr& ev) noexcept;

  bool send_response(Buffers::Ptr cbuf,
                     DispatchHandler::Ptr hdlr=nullptr) noexcept;

  bool send_request(Buffers::Ptr cbuf, DispatchHandler::Ptr hdlr);

  void print(std::ostream& out) const;

  protected:

  SWC_CAN_INLINE
  ConnHandler(AppContext::Ptr& a_app_ctx) noexcept
              : connected(true), app_ctx(a_app_ctx),
                endpoint_remote(), endpoint_local(),
                m_mutex(), m_next_req_id(0),
                m_outgoing(), m_pending(), m_recv_bytes(0) {
  }

  SWC_CAN_INLINE
  ConnHandlerPtr ptr() noexcept {
    return shared_from_this();
  }

  virtual ~ConnHandler() noexcept { }

  virtual SocketLayer* socket_layer() noexcept = 0;

  virtual void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) = 0;


  struct Sender_noAck;
  struct Sender_Ack;

  virtual void do_async_write(
      Core::Vector<asio::const_buffer>&& buffers, Sender_noAck&& hdlr)
      noexcept = 0;
  virtual void do_async_write(
      Core::Vector<asio::const_buffer>&& buffers, Sender_Ack&& hdlr)
      noexcept = 0;


  struct Receiver_HeaderPrefix;
  struct Receiver_Header;
  struct Receiver_Buffer;

  virtual void do_async_read(uint8_t* data, uint32_t sz,
                             Receiver_HeaderPrefix&& hdlr) noexcept = 0;
  virtual void do_async_read(uint8_t* data, uint32_t sz,
                             Receiver_Header&& hdlr) noexcept = 0;
  virtual void do_async_read(uint8_t* data, uint32_t sz,
                             Receiver_Buffer&& hdlr) noexcept = 0;


  void do_close_run() noexcept;

  void disconnected() noexcept;

  Core::MutexSptd m_mutex;

  private:

  void write_or_queue(Outgoing&& outgoing);

  void write_next();

  void write(Outgoing& outgoing);

  void read() noexcept;

  void recv_buffers(Event::Ptr&& ev);

  void received(Event::Ptr&& ev) noexcept;

  SWC_CAN_INLINE
  void do_close_recv() noexcept {
    if(m_recv_bytes)
      app_ctx->net_bytes_received(ptr(), m_recv_bytes);
    do_close();
  }

  void run_pending(Event::Ptr&& ev);

  struct PendingHash {
    SWC_CAN_INLINE
    size_t operator()(const uint32_t id) const {
      return id >> 12;
    }
  };

  uint32_t                          m_next_req_id;
  Core::QueueSafeStated<Outgoing>   m_outgoing;
  std::unordered_map<uint32_t,
                     Pending,
                     PendingHash>   m_pending;
  size_t                            m_recv_bytes;
  uint8_t _buf_header[Header::MAX_LENGTH];
};





class ConnHandlerPlain final : public ConnHandler {
  public:

  typedef std::shared_ptr<ConnHandlerPlain> Ptr;

  static Ptr make(AppContext::Ptr& app_ctx, SocketPlain& socket);

  virtual ~ConnHandlerPlain() noexcept;

  void do_close() noexcept override;

  bool is_open() const noexcept override;

  protected:

  SocketLayer* SWC_CONST_FUNC socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    Core::Vector<asio::const_buffer>&& buffers, Sender_noAck&& hdlr)
    noexcept override;
  void do_async_write(
    Core::Vector<asio::const_buffer>&& buffers, Sender_Ack&& hdlr)
    noexcept override;

  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_HeaderPrefix&& hdlr) noexcept override;
  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_Header&& hdlr) noexcept override;
  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_Buffer&& hdlr) noexcept override;

  private:

  ConnHandlerPlain(AppContext::Ptr& app_ctx, SocketPlain& socket) noexcept;

  SocketPlain  m_sock;

};



class ConnHandlerSSL final : public ConnHandler {
  public:

  typedef std::shared_ptr<ConnHandlerSSL> Ptr;

  static Ptr make(AppContext::Ptr& app_ctx, asio::ssl::context& ssl_ctx,
                  SocketPlain& socket);

  virtual ~ConnHandlerSSL() noexcept;

  bool is_secure() const noexcept override { return true; }

  void do_close() noexcept override;

  bool is_open() const noexcept override;

  template<typename T>
  void set_verify(T&& hdlr) noexcept {
    m_sock.set_verify_callback(std::move(hdlr));
  }

  template<typename T>
  void handshake(SocketSSL::handshake_type typ, T&& hdlr) noexcept {
    /* any options ?
    asio::error_code ec;
    m_sock.lowest_layer().non_blocking(true, ec);
    if(ec)
      hdlr(ec);
    else
    */
    m_sock.async_handshake(typ, std::move(hdlr));
  }

  void handshake(SocketSSL::handshake_type typ,
                 asio::error_code& ec) noexcept;

  protected:

  SocketLayer* SWC_CONST_FUNC socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    Core::Vector<asio::const_buffer>&& buffers, Sender_noAck&& hdlr)
    noexcept override;
  void do_async_write(
    Core::Vector<asio::const_buffer>&& buffers, Sender_Ack&& hdlr)
    noexcept override;

  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_HeaderPrefix&& hdlr) noexcept override;
  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_Header&& hdlr) noexcept override;
  void do_async_read(uint8_t* data, uint32_t sz,
                     Receiver_Buffer&& hdlr) noexcept override;

  private:

  ConnHandlerSSL(AppContext::Ptr& app_ctx, asio::ssl::context& ssl_ctx,
                 SocketPlain& socket) noexcept;

  SocketSSL                               m_sock;
  asio::strand<SocketSSL::executor_type>  m_strand;

};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConnHandler.cc"
#endif

#endif // swcdb_core_comm_ConnHandler_h