/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_core_comm_ConnHandler_h
#define swc_core_comm_ConnHandler_h

#include <asio.hpp>
#include "asio/ssl.hpp"
#include <memory>
#include <mutex>
#include "swcdb/core/Error.h"
#include "swcdb/core/QueueSafeStated.h"
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/CommBuf.h"
#include "swcdb/core/comm/Resolver.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/DispatchHandler.h"


namespace SWC { 

using SocketPlain = asio::ip::tcp::socket;
using SocketSSL = asio::ssl::stream<asio::ip::tcp::socket>;


class ConnHandler : public std::enable_shared_from_this<ConnHandler> {

  struct PendingRsp final {
    public:
    PendingRsp(DispatchHandler::Ptr hdlr, asio::high_resolution_timer* tm);
    ~PendingRsp();
    DispatchHandler::Ptr          hdlr;
    asio::high_resolution_timer*  tm;
  };

  struct Outgoing final {
    public:
    Outgoing(CommBuf::Ptr cbuf, DispatchHandler::Ptr hdlr);
    ~Outgoing();
    CommBuf::Ptr          cbuf;
    DispatchHandler::Ptr  hdlr; 
  };

  public:
  
  std::atomic<bool>     connected;
  const AppContext::Ptr app_ctx;
  EndPoint              endpoint_remote;
  EndPoint              endpoint_local;

  ConnHandler(AppContext::Ptr app_ctx);

  ConnHandlerPtr ptr();

  virtual ~ConnHandler();

  std::string endpoint_local_str();
  
  std::string endpoint_remote_str();
  
  size_t endpoint_remote_hash();
  
  size_t endpoint_local_hash();
  
  virtual void new_connection();

  virtual bool is_open() = 0;

  virtual void close() = 0;

  size_t pending_read();

  size_t pending_write();

  bool due();

  virtual void run(Event::Ptr& ev);

  virtual void do_close();

  bool send_error(int error, const std::string &msg, 
                  const Event::Ptr& ev=nullptr);

  bool response_ok(const Event::Ptr& ev=nullptr);

  bool send_response(CommBuf::Ptr &cbuf, DispatchHandler::Ptr hdlr=nullptr);

  bool send_request(CommBuf::Ptr &cbuf, DispatchHandler::Ptr hdlr);

  void accept_requests();

  /* 
  void accept_requests(DispatchHandler::Ptr hdlr, uint32_t timeout_ms=0);
  */

  std::string to_string();

  protected:

  virtual asio::high_resolution_timer* get_timer(uint32_t timeout_ms) = 0;

  virtual void read(uint8_t** bufp, size_t* remainp, asio::error_code &ec) = 0;

  virtual void do_async_write(
      const std::vector<asio::const_buffer>& buffers,
      const std::function<void(const asio::error_code, uint32_t)>&) = 0;

  virtual void do_async_read(
      uint8_t* data, uint32_t sz,
      const std::function<void(const asio::error_code, size_t)>& hdlr) = 0;

  void disconnected();

  std::mutex  m_mutex;

  private:

  uint32_t next_req_id();

  asio::high_resolution_timer* get_timer(const CommHeader& header);
    
  void write_or_queue(Outgoing* data);
  
  void pending(ConnHandler::Outgoing* data, asio::high_resolution_timer* tm);

  void next_outgoing();

  void clear_outgoing();

  void write(Outgoing* data);

  void read_pending();

  void recved_header_pre(asio::error_code ec, 
                         const uint8_t* data, size_t filled);

  void recved_header(const Event::Ptr& ev, asio::error_code ec, 
                     const uint8_t* data, size_t filled);

  void recv_buffers(const Event::Ptr& ev, uint8_t n);

  void recved_buffer(const Event::Ptr& ev, asio::error_code ec, 
                     uint8_t n, size_t filled);

  void received(const Event::Ptr& ev, const asio::error_code& ec);

  void run_pending(Event::Ptr ev);

  uint32_t                    m_next_req_id;
  QueueSafeStated<Outgoing*>  m_outgoing;

  bool                        m_accepting = 0;
  bool                        m_reading = 0;
  std::unordered_map<uint32_t, PendingRsp*>  m_pending;

};




class ConnHandlerPlain : public ConnHandler {
  public:

  ConnHandlerPlain(AppContext::Ptr app_ctx, SocketPlain& socket);
  
  virtual ~ConnHandlerPlain();

  void do_close() override;

  void close() override;

  void new_connection() override;

  bool is_open() override;

  asio::high_resolution_timer* get_timer(uint32_t timeout_ms) override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code &ec) override;

  void do_async_write(
    const std::vector<asio::const_buffer>& buffers,
    const std::function<void(const asio::error_code, uint32_t)>& hdlr) override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    const std::function<void(const asio::error_code, size_t)>& hdlr) override;

  private:
  SocketPlain  m_sock;

};


class ConnHandlerSSL : public ConnHandler {
  public:

  ConnHandlerSSL(AppContext::Ptr app_ctx, asio::ssl::context& ssl_ctx, 
                 SocketPlain& socket);
  
  virtual ~ConnHandlerSSL();

  void do_close() override;

  void close() override;

  void new_connection() override;

  bool is_open() override;

  void handshake();

  void set_verify(
    const std::function<bool(bool, asio::ssl::verify_context&)>& cb);

  void handshake_client(const std::function<void(const asio::error_code&)> cb);

  void handshake_client(asio::error_code& ec);

  asio::high_resolution_timer* get_timer(uint32_t timeout_ms) override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code &ec) override;

  void do_async_write(
    const std::vector<asio::const_buffer>& buffers,
    const std::function<void(const asio::error_code, uint32_t)>& hdlr) override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    const std::function<void(const asio::error_code, size_t)>& hdlr) override;

  private:
  SocketSSL  m_sock;

};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConnHandler.cc"
#endif 

#endif // swc_core_comm_ConnHandler_h