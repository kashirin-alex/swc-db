/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandler_h
#define swc_core_comm_ConnHandler_h

#include <asio.hpp>
#include <queue>
#include <memory>
#include "swcdb/core/Error.h"
#include "swcdb/core/LockAtomicUnique.h"
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/CommBuf.h"
#include "swcdb/core/comm/Resolver.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/DispatchHandler.h"


namespace SWC { 

using Socket = asio::ip::tcp::socket;

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

  ConnHandler(AppContext::Ptr app_ctx, Socket& socket);

  ConnHandlerPtr ptr();

  virtual ~ConnHandler();

  const std::string endpoint_local_str();
  
  const std::string endpoint_remote_str();
  
  const size_t endpoint_remote_hash();
  
  const size_t endpoint_local_hash();
  
  virtual void new_connection();

  const bool is_open();

  void close();

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

  /*
  int send_response(CommBuf::Ptr &cbuf, uint32_t timeout_ms);

  int send_request(uint32_t timeout_ms, CommBuf::Ptr &cbuf, 
                          DispatchHandler::Ptr hdlr);
  */

  const int send_request(CommBuf::Ptr &cbuf, DispatchHandler::Ptr hdlr);

  void accept_requests();
  /* 
  void accept_requests(DispatchHandler::Ptr hdlr, uint32_t timeout_ms=0);
  */

  const std::string to_string();

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

  void disconnected();

  void run_pending(Event::Ptr ev);

  Socket                    m_sock;
  uint32_t                  m_next_req_id;

  LockAtomic::Unique        m_mutex;
  std::queue<Outgoing*>     m_outgoing;
  bool                      m_writing = 0;

  LockAtomic::Unique        m_mutex_reading;
  bool                      m_accepting = 0;
  bool                      m_reading = 0;
  std::unordered_map<uint32_t, PendingRsp*>  m_pending;

  std::atomic<Error::Code>  m_err = Error::OK;

};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/comm/ConnHandler.cc"
#endif 

#endif // swc_core_comm_ConnHandler_h