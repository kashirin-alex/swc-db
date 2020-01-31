/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandler_h
#define swc_core_comm_ConnHandler_h

#include <asio.hpp>
//#include "asio/ssl.hpp"
#include <queue>
#include <memory>
#include <mutex>
#include "swcdb/core/Error.h"
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/CommBuf.h"
#include "swcdb/core/comm/Resolver.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/DispatchHandler.h"


namespace SWC { 

using SocketPlain = asio::ip::tcp::socket;
//using SocketSSL = asio::ssl::stream<asio::ip::tcp::socket>;


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
  
  const AppContext::Ptr app_ctx;
  EndPoint              endpoint_remote;
  EndPoint              endpoint_local;

  ConnHandler(AppContext::Ptr app_ctx);

  ConnHandlerPtr ptr();

  virtual ~ConnHandler();

  const std::string endpoint_local_str();
  
  const std::string endpoint_remote_str();
  
  const size_t endpoint_remote_hash();
  
  const size_t endpoint_local_hash();
  
  virtual void new_connection();

  virtual const bool is_open() = 0;

  virtual void close() = 0;

  const size_t pending_read();

  const size_t pending_write();

  const bool due();

  virtual void run(Event::Ptr& ev);

  void do_close();

  const int send_error(int error, const std::string &msg, 
                       const Event::Ptr& ev=nullptr);

  const int response_ok(const Event::Ptr& ev=nullptr);

  const int send_response(CommBuf::Ptr &cbuf, 
                          DispatchHandler::Ptr hdlr=nullptr);

  const int send_request(CommBuf::Ptr &cbuf, DispatchHandler::Ptr hdlr);

  void accept_requests();

  /* 
  void accept_requests(DispatchHandler::Ptr hdlr, uint32_t timeout_ms=0);
  */

  const std::string to_string();

  protected:

  virtual asio::high_resolution_timer* get_timer(uint32_t timeout_ms) = 0;

  virtual void read(uint8_t** bufp, size_t* remainp, asio::error_code &ec) = 0;

  virtual void do_async_write(
      const std::vector<asio::const_buffer>& buffers,
      const std::function<void(const asio::error_code, uint32_t)>&) = 0;

  virtual void do_async_read(
      uint8_t* data, uint32_t sz,
      const std::function<size_t(const asio::error_code, size_t)>& cond,
      const std::function<void(const asio::error_code, size_t)>& hdlr) = 0;

  void disconnected();

  std::mutex                m_mutex;
  std::atomic<Error::Code>  m_err = Error::OK;

  private:

  const uint32_t next_req_id();

  asio::high_resolution_timer* get_timer(const CommHeader& header);
    
  void write_or_queue(Outgoing* data);
  
  void next_outgoing();
  
  void write(Outgoing* data);

  void read_pending();
  
  size_t read_condition_hdlr(const Event::Ptr& ev, uint8_t* data,
                             const asio::error_code e, size_t filled);
  
  size_t read_condition(const Event::Ptr& ev, uint8_t* data, 
                        asio::error_code &ec);

  void received(const Event::Ptr& ev, const asio::error_code ec);

  void run_pending(Event::Ptr ev);

  uint32_t                  m_next_req_id;

  std::queue<Outgoing*>     m_outgoing;
  bool                      m_writing = 0;

  std::mutex                m_mutex_reading;
  bool                      m_accepting = 0;
  bool                      m_reading = 0;
  std::unordered_map<uint32_t, PendingRsp*>  m_pending;

};




class ConnHandlerPlain : public ConnHandler {
  public:

  ConnHandlerPlain(AppContext::Ptr app_ctx, SocketPlain& socket);
  
  virtual ~ConnHandlerPlain();

  void new_connection() override;

  const bool is_open() override;

  void close() override;

  asio::high_resolution_timer* get_timer(uint32_t timeout_ms) override;

  void read(uint8_t** bufp, size_t* remainp, asio::error_code &ec) override;

  void do_async_write(
    const std::vector<asio::const_buffer>& buffers,
    const std::function<void(const asio::error_code, uint32_t)>& hdlr) override;

  void do_async_read(
    uint8_t* data, uint32_t sz,
    const std::function<size_t(const asio::error_code, size_t)>& cond,
    const std::function<void(const asio::error_code, size_t)>& hdlr) override;

  private:
  SocketPlain  m_sock;

};




} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConnHandler.cc"
#endif 

#endif // swc_core_comm_ConnHandler_h