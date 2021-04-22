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
    
    Outgoing() noexcept : cbuf(nullptr), hdlr(nullptr) { }
    
    Outgoing(Buffers::Ptr&& cbuf, DispatchHandler::Ptr&& hdlr) noexcept
            : cbuf(std::move(cbuf)), hdlr(std::move(hdlr)) {
    }
    
    Outgoing(Outgoing&& other) noexcept
           : cbuf(std::move(other.cbuf)), hdlr(std::move(other.hdlr)) {
    }
    
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

    Pending() noexcept : hdlr(nullptr), timer(nullptr) { }

    Pending(DispatchHandler::Ptr&& hdlr) noexcept
           : hdlr(std::move(hdlr)), timer(nullptr) {
    }

    Pending(Pending&& other) noexcept
           : hdlr(std::move(other.hdlr)), timer(other.timer) {
      other.timer = nullptr;
    }

    Pending& operator=(Pending&& other) noexcept {
      hdlr = std::move(other.hdlr);
      timer = other.timer;
      other.timer = nullptr;
      return *this;
    }

    Pending(const Pending&)             = delete;

    Pending& operator=(const Pending&)  = delete;

    ~Pending() {
      if(timer)
        delete timer;
    }
  };


  public:

  Core::AtomicBool      connected;
  const AppContext::Ptr app_ctx;
  EndPoint              endpoint_remote;
  EndPoint              endpoint_local;

  ConnHandler(AppContext::Ptr& app_ctx) noexcept
              : connected(true), app_ctx(app_ctx), m_next_req_id(0) {
  }

  ConnHandlerPtr ptr() noexcept {
    return shared_from_this();
  }

  size_t endpoint_remote_hash() const;

  size_t endpoint_local_hash() const;

  Core::Encoder::Type get_encoder() const noexcept;

  virtual bool is_secure() const noexcept { return false; };

  void new_connection();

  virtual bool is_open() const noexcept = 0;

  size_t pending_read() noexcept;

  size_t pending_write();

  bool due();

  void run(const Event::Ptr& ev);

  virtual void do_close() = 0;

  void do_close_run();

  bool send_error(int error, const std::string& msg,
                  const Event::Ptr& ev) noexcept;

  bool response_ok(const Event::Ptr& ev) noexcept;

  bool send_response(Buffers::Ptr cbuf,
                     DispatchHandler::Ptr hdlr=nullptr) noexcept;

  bool send_request(Buffers::Ptr cbuf, DispatchHandler::Ptr hdlr);

  void print(std::ostream& out) const;

  protected:

  virtual ~ConnHandler() { }

  virtual SocketLayer* socket_layer() noexcept = 0;

  virtual void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) = 0;

  virtual void do_async_write(
      std::vector<asio::const_buffer>&& buffers,
      std::function<void(const asio::error_code&, uint32_t)>&& hdlr)
      noexcept = 0;

  virtual void do_async_read(
      uint8_t* data, uint32_t sz,
      std::function<void(const asio::error_code&, size_t)>&& hdlr)
      noexcept = 0;

  void disconnected();

  Core::MutexSptd m_mutex;

  private:

  void write_or_queue(Outgoing&& outgoing);

  void write_next();

  void write(Outgoing& outgoing);

  void read();

  void recved_header_pre(const asio::error_code& ec, size_t filled);

  void recved_header(const Event::Ptr& ev, asio::error_code ec,
                     size_t filled);

  void recv_buffers(const Event::Ptr& ev, uint8_t n);

  void recved_buffer(const Event::Ptr& ev, asio::error_code ec,
                     uint8_t n, size_t filled);

  void received(const Event::Ptr& ev);

  void run_pending(const Event::Ptr& ev);

  struct PendingHash {
    size_t operator()(const uint32_t id) const {
      return id >> 12;
    }
  };

  uint32_t                          m_next_req_id;
  Core::QueueSafeStated<Outgoing>   m_outgoing;
  std::unordered_map<uint32_t,
                     Pending,
                     PendingHash>   m_pending;
  uint8_t _buf_header[Header::MAX_LENGTH];
};




class ConnHandlerPlain final : public ConnHandler {
  public:

  ConnHandlerPlain(AppContext::Ptr& app_ctx, SocketPlain& socket) noexcept;

  virtual ~ConnHandlerPlain();

  void do_close() override;

  bool is_open() const noexcept override;

  protected:

  SocketLayer* socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    std::vector<asio::const_buffer>&& buffers,
    std::function<void(const asio::error_code&, uint32_t)>&& hdlr)
    noexcept override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    std::function<void(const asio::error_code&, size_t)>&& hdlr)
    noexcept override;

  private:
  SocketPlain  m_sock;

};


class ConnHandlerSSL final : public ConnHandler {
  public:

  ConnHandlerSSL(AppContext::Ptr& app_ctx, asio::ssl::context& ssl_ctx,
                 SocketPlain& socket) noexcept;

  virtual ~ConnHandlerSSL();

  bool is_secure() const noexcept override { return true; }

  void do_close() override;

  bool is_open() const noexcept override;

  void handshake(SocketSSL::handshake_type typ,
                 std::function<void(const asio::error_code&)>&& cb)
                 noexcept;

  void handshake(SocketSSL::handshake_type typ,
                 asio::error_code& ec) noexcept;

  void set_verify(
    std::function<bool(bool, asio::ssl::verify_context&)>&& cb)
    noexcept;

  protected:

  SocketLayer* socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    std::vector<asio::const_buffer>&& buffers,
    std::function<void(const asio::error_code&, uint32_t)>&& hdlr)
    noexcept override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    std::function<void(const asio::error_code&, size_t)>&& hdlr)
    noexcept override;

  private:
  SocketSSL                               m_sock;
  asio::strand<SocketSSL::executor_type>  m_strand;

};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConnHandler.cc"
#endif

#endif // swcdb_core_comm_ConnHandler_h