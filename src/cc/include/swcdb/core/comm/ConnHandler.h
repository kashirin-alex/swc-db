/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_ConnHandler_h
#define swcdb_core_comm_ConnHandler_h


#include "swcdb/core/Exception.h"
#include <asio.hpp>
#include "asio/ssl.hpp"
#include <memory>
#include <mutex>
#include "swcdb/core/StateRunning.h"
#include "swcdb/core/QueueSafeStated.h"
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


  struct Pending {
    Buffers::Ptr                  cbuf;
    DispatchHandler::Ptr          hdlr;
    asio::high_resolution_timer*  timer;

    Pending(const Buffers::Ptr& cbuf, DispatchHandler::Ptr& hdlr) noexcept;

    Pending(const Pending&) = delete;

    Pending(const Pending&&) = delete;

    Pending& operator=(const Pending&) = delete;

    ~Pending();
  };


  public:

  Core::AtomicBool      connected;
  const AppContext::Ptr app_ctx;
  EndPoint              endpoint_remote;
  EndPoint              endpoint_local;

  ConnHandler(AppContext::Ptr& app_ctx) noexcept;

  ConnHandlerPtr ptr() noexcept;

  virtual ~ConnHandler();

  size_t endpoint_remote_hash() const;

  size_t endpoint_local_hash() const;

  Core::Encoder::Type get_encoder() const noexcept;

  virtual bool is_secure() const noexcept { return false; };

  void new_connection();

  virtual bool is_open() const noexcept = 0;

  virtual void close() = 0;

  size_t pending_read();

  size_t pending_write();

  bool due();

  void run(const Event::Ptr& ev);

  virtual void do_close() = 0;

  void do_close_run();

  bool send_error(int error, const std::string& msg,
                  const Event::Ptr& ev) noexcept;

  bool response_ok(const Event::Ptr& ev) noexcept;

  bool send_response(const Buffers::Ptr& cbuf,
                     DispatchHandler::Ptr hdlr=nullptr) noexcept;

  bool send_request(Buffers::Ptr& cbuf, DispatchHandler::Ptr hdlr);

  void accept_requests();

  /*
  void accept_requests(DispatchHandler::Ptr hdlr, uint32_t timeout_ms=0);
  */

  void print(std::ostream& out) const;

  protected:

  virtual SocketLayer* socket_layer() noexcept = 0;

  virtual void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) = 0;

  virtual void do_async_write(
      const std::vector<asio::const_buffer>& buffers,
      const std::function<void(const asio::error_code&, uint32_t)>&)
      noexcept = 0;

  virtual void do_async_read(
      uint8_t* data, uint32_t sz,
      const std::function<void(const asio::error_code&, size_t)>& hdlr)
      noexcept = 0;

  void disconnected();

  Core::MutexSptd m_mutex;

  private:

  void write_or_queue(Pending* pending);

  void write_next();

  void write(Pending* pending);

  void read_pending();

  void recved_header_pre(const asio::error_code& ec,
                         const uint8_t* data, size_t filled);

  void recved_header(const Event::Ptr& ev, asio::error_code ec,
                     const uint8_t* data, size_t filled);

  void recv_buffers(const Event::Ptr& ev, uint8_t n);

  void recved_buffer(const Event::Ptr& ev, asio::error_code ec,
                     uint8_t n, size_t filled);

  void received(const Event::Ptr& ev);

  void run_pending(const Event::Ptr& ev);

  struct PendingHash {
    size_t operator()(const uint32_t id) const {
      return id / 4096;
    }
  };

  uint32_t                          m_next_req_id;
  Core::AtomicBool                  m_accepting;
  Core::StateRunning                m_read;
  Core::QueueSafeStated<Pending*>   m_outgoing;
  std::unordered_map<uint32_t,
                    Pending*,
                    PendingHash>    m_pending;
};




class ConnHandlerPlain final : public ConnHandler {
  public:

  ConnHandlerPlain(AppContext::Ptr& app_ctx, SocketPlain& socket) noexcept;

  virtual ~ConnHandlerPlain();

  void do_close() override;

  void close() override;

  bool is_open() const noexcept override;

  protected:

  SocketLayer* socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    const std::vector<asio::const_buffer>& buffers,
    const std::function<void(const asio::error_code&, uint32_t)>& hdlr)
    noexcept override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    const std::function<void(const asio::error_code&, size_t)>& hdlr)
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

  void close() override;

  bool is_open() const noexcept override;

  void handshake(SocketSSL::handshake_type typ,
                 const std::function<void(const asio::error_code&)>& cb)
                 noexcept;

  void handshake(SocketSSL::handshake_type typ,
                 asio::error_code& ec) noexcept;

  void set_verify(
    const std::function<bool(bool, asio::ssl::verify_context&)>& cb)
    noexcept;

  protected:

  SocketLayer* socket_layer() noexcept override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code& ec) override;

  void do_async_write(
    const std::vector<asio::const_buffer>& buffers,
    const std::function<void(const asio::error_code&, uint32_t)>& hdlr)
    noexcept override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    const std::function<void(const asio::error_code&, size_t)>& hdlr)
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