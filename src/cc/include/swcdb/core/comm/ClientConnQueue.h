/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_comm_ClientConnQueue_h
#define swc_core_comm_ClientConnQueue_h

#include <queue>
#include <unordered_set>
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client {


class ConnQueue;
typedef std::shared_ptr<ConnQueue> ConnQueuePtr;

class ConnQueueReqBase : public DispatchHandler {
  public:

  typedef std::shared_ptr<ConnQueueReqBase> Ptr;

  ConnQueueReqBase(bool insistent=true, CommBuf::Ptr cbp=nullptr);

  Ptr req();

  virtual ~ConnQueueReqBase();

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  bool is_timeout(ConnHandlerPtr conn, Event::Ptr& ev);

  bool is_rsp(ConnHandlerPtr conn, Event::Ptr& ev);

  void request_again();

  virtual bool valid();

  virtual void handle_no_conn();

   std::string to_string();
    
  const bool            insistent;
  CommBuf::Ptr          cbp;
  std::atomic<bool>     was_called;
  ConnQueuePtr          queue;
};



class ConnQueue : 
    private std::queue<ConnQueueReqBase::Ptr>, 
    public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef ConnQueueReqBase ReqBase;

  ConnQueue(IOCtxPtr ioctx,
            const Property::V_GINT32::Ptr keepalive_ms=nullptr, 
            const Property::V_GINT32::Ptr again_delay_ms=nullptr);

  virtual ~ConnQueue();

  virtual bool connect();

  virtual void close_issued();

  void stop();

  void put(ReqBase::Ptr req);

  void set(const ConnHandlerPtr& conn);

  void delay(ReqBase::Ptr req);

  void delay_proceed(const ReqBase::Ptr& req, asio::high_resolution_timer* tm);

  std::string to_string();

  private:
  
  void exec_queue();

  void run_queue();

  void schedule_close();

  Mutex                                             m_mutex;
  IOCtxPtr                                          m_ioctx;
  ConnHandlerPtr                                    m_conn;
  bool                                              m_connecting;
  bool                                              m_qrunning;
  asio::high_resolution_timer*                      m_timer; 
  std::unordered_set<asio::high_resolution_timer*>  m_delayed;

  protected:
  const Property::V_GINT32::Ptr  cfg_keepalive_ms;
  const Property::V_GINT32::Ptr  cfg_again_delay_ms;

};


}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ClientConnQueue.cc"
#endif 


// 
#endif // swc_core_comm_ClientConnQueue_h