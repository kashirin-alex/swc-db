/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_ClientConnQueue_h
#define swc_core_comm_ClientConnQueue_h

#include <queue>

#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client {


class ConnQueue : public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef std::shared_ptr<ConnQueue> Ptr;

  class ReqBase : public DispatchHandler {
    public:

    typedef std::shared_ptr<ReqBase> Ptr;

    ReqBase(bool insistent=true, CommBuf::Ptr cbp=nullptr);

    Ptr req();

    virtual ~ReqBase();

    void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

    bool is_timeout(ConnHandlerPtr conn, Event::Ptr& ev);

    bool is_rsp(ConnHandlerPtr conn, Event::Ptr& ev);

    void request_again();

    virtual bool valid();

    virtual void handle_no_conn();

    const std::string to_string();
    
    const bool            insistent;
    CommBuf::Ptr          cbp;
    std::atomic<bool>     was_called;
    ConnQueue::Ptr        queue;
  };


  ConnQueue(IOCtxPtr ioctx,
            const Property::V_GINT32::Ptr keepalive_ms=nullptr, 
            const Property::V_GINT32::Ptr again_delay_ms=nullptr);

  virtual ~ConnQueue();

  virtual bool connect();

  virtual void close_issued();

  void stop();

  void put(ReqBase::Ptr req);

  void set(ConnHandlerPtr conn);

  void delay(ReqBase::Ptr req);

  void delay_proceed(ReqBase::Ptr req, asio::high_resolution_timer* tm);

  const std::string to_string();

  private:
  
  void exec_queue();

  void run_queue();

  void schedule_close();

  std::recursive_mutex          m_mutex;
  std::queue<ReqBase::Ptr>      m_queue;
  ConnHandlerPtr                m_conn;
  bool                          m_queue_running;
  bool                          m_connecting;
  IOCtxPtr                      m_ioctx;
  asio::high_resolution_timer*  m_timer; 
  std::vector<asio::high_resolution_timer*>  m_delayed;

  protected:
  const Property::V_GINT32::Ptr  cfg_keepalive_ms;
  const Property::V_GINT32::Ptr  cfg_again_delay_ms;

};


}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ClientConnQueues.cc"
#endif 


// 
#endif // swc_core_comm_ClientConnQueue_h