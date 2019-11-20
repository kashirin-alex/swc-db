/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_commmon_req_ConnQueue_h
#define swc_lib_db_protocol_commmon_req_ConnQueue_h

#include <queue>

namespace SWC { namespace Protocol { namespace Common { namespace Req {


class ConnQueue : public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef std::shared_ptr<ConnQueue> Ptr;

  class ReqBase : public DispatchHandler {
    public:

    typedef std::shared_ptr<ReqBase> Ptr;

    ReqBase(bool insistent=true, CommBuf::Ptr cbp=nullptr) 
            : insistent(insistent), cbp(cbp), 
              was_called(false), queue(nullptr){
    }

    inline Ptr req() {
      return std::dynamic_pointer_cast<ReqBase>(shared_from_this());
    }

    virtual ~ReqBase() {}

    void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
      if(was_called || !is_rsp(conn, ev))
        return;
      // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    }

    bool is_timeout(ConnHandlerPtr conn, Event::Ptr& ev) {
      bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
      if(out)
        request_again();
      return out;
    }

    bool is_rsp(ConnHandlerPtr conn, Event::Ptr& ev){
      if(ev->type == Event::Type::DISCONNECT 
        || ev->error == Error::Code::REQUEST_TIMEOUT){

        if(!was_called)
          request_again();
        return false;
      }
      return true;
    }

    void request_again() {
      queue->put(req());
    }

    virtual bool valid() { return true; }

    virtual void handle_no_conn() {}

    const std::string to_string(){
      std::string s("ReqBase ");
      s.append(" called=");
      s.append(std::to_string(was_called.load()));
      s.append(" insistent=");
      s.append(std::to_string(insistent));
      s.append(" command=");
      s.append(std::to_string((int64_t)cbp->header.command));
      return s;
    }
    
    const bool            insistent;
    CommBuf::Ptr          cbp;
    std::atomic<bool>     was_called;
    ConnQueue::Ptr        queue;
  };


  ConnQueue(const gInt32tPtr keepalive_ms=nullptr) 
            : m_conn(nullptr),  m_queue_running(false), m_connecting(false),
              cfg_keepalive_ms(keepalive_ms), m_timer(nullptr) { 
  }

  virtual ~ConnQueue() {
    if(m_timer)
      delete m_timer;
  }

  virtual bool connect() { 
    return false; // not implemented by default 
  }

  virtual void close_issued() { }

  void stop() {
    ReqBase::Ptr req;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if(m_conn != nullptr && m_conn->is_open())
      m_conn->do_close();

    if(m_timer != nullptr) {
      m_timer->cancel();
      m_timer = nullptr;
    }

    while(!m_queue.empty()) {
      req = m_queue.front();
      m_queue.pop();
      if(!req->was_called)
        req->handle_no_conn();
    }
  }

  void put(ReqBase::Ptr req){
    if(req->queue == nullptr) 
      req->queue = shared_from_this();
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      m_queue.push(req);
      if(m_conn == nullptr || !m_conn->is_open())
        if(m_connecting)
          return;
        m_connecting = true;
        if(connect())
          return;
    }
    exec_queue();
  }

  void set(ConnHandlerPtr conn){
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      m_conn = conn;
      m_connecting = false;
    }
    exec_queue();
  }

  const std::string to_string() {
    std::string s("ConnQueue:");
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    s.append(" size=");
    s.append(std::to_string(m_queue.size()));
    s.append(" conn=");
    s.append(m_conn!=nullptr?m_conn->endpoint_remote_str():std::string("no"));
    return s;
  }

  private:
  
  void exec_queue(){
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      if(m_queue_running)
        return;
      m_queue_running = true;
    }

    asio::post(*Env::IoCtx::io()->ptr(), 
               [ptr=shared_from_this()](){ptr->run_queue();});
  }

  void run_queue(){
    { 
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      if(m_timer != nullptr) {
        m_timer->cancel();
        m_timer = nullptr;
      }
    }
    ReqBase::Ptr    req;
    ConnHandlerPtr  conn;
    bool sent;
    for(;;) {
      {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if(m_queue.empty()){
          m_queue_running = false;
          break;
        } 
        req = m_queue.front();
          if((m_conn == nullptr || !m_conn->is_open()) && req->insistent) {
          m_queue_running = false;
          break;
        }
        conn = m_conn;
      }
      
      HT_ASSERT(req->cbp != nullptr);

      if(!req->valid())
        goto processed;
        
      if(sent = (conn != nullptr && conn->send_request(req->cbp, req) == Error::OK))
        goto processed;

      req->handle_no_conn();
      if(req->insistent)
        continue;
        
      processed: {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_queue.pop();
        m_queue_running = !m_queue.empty();
        if(m_queue_running) 
          continue;
        break;
      }
  
    }

    schedule_close();
  }

  void schedule_close() {
    if(cfg_keepalive_ms == nullptr) // nullptr -eq persistent
      return;
    // ~ on timer after ms+ OR socket_opt ka(0)+interval(ms+) 

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    if(m_queue.empty() && (m_conn == nullptr || !m_conn->due())) {
      if(m_conn != nullptr)
        m_conn->do_close(); 
      if(m_timer != nullptr) {
        m_timer->cancel();
        m_timer = nullptr;
      }
      close_issued();
      return;
    }
    
    if(m_timer == nullptr)
      m_timer = new asio::high_resolution_timer(*Env::IoCtx::io()->ptr());
    else
      m_timer->cancel();

    m_timer->expires_from_now(
      std::chrono::milliseconds(cfg_keepalive_ms->get()));
    m_timer->async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->schedule_close();
        }
      }
    );   
  }

  std::recursive_mutex          m_mutex;
  std::queue<ReqBase::Ptr>      m_queue;
  ConnHandlerPtr                m_conn;
  bool                          m_queue_running;
  bool                          m_connecting;
  asio::high_resolution_timer*  m_timer; 

  protected:
  const gInt32tPtr          cfg_keepalive_ms;
};


}}}}

#endif // swc_lib_db_protocol_commmon_req_ConnQueue_h