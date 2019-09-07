/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_ConnQueue_h
#define swc_lib_db_protocol_req_ConnQueue_h

#include <queue>

namespace SWC { namespace Protocol { namespace Req {


class ConnQueue : public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef std::shared_ptr<ConnQueue> Ptr;

  class ReqBase : public DispatchHandler {
    public:

    typedef std::shared_ptr<ReqBase> Ptr;

    ReqBase(bool insistent=true) 
            : cbp(nullptr), was_called(false), queue(nullptr), 
              insistent(insistent) {}

    virtual ~ReqBase() {}

    void handle(ConnHandlerPtr conn, EventPtr &ev) {
      if(was_called || !is_rsp(conn, ev))
        return;
      // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    }

    bool is_timeout(ConnHandlerPtr conn, EventPtr &ev) {
      bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
      if(out)
        request_again();
      return out;
    }

    bool is_rsp(ConnHandlerPtr conn, EventPtr &ev){
      if(ev->type == Event::Type::DISCONNECT 
        || ev->error == Error::Code::REQUEST_TIMEOUT){

        if(!was_called)
          request_again();
        return false;
      }
      return true;
    }

    void request_again() {
      std::cout << "ConnQueue request_again \n";
      queue->put(std::dynamic_pointer_cast<ReqBase>(shared_from_this()));
    }

    virtual bool valid() { return true; }

    virtual void handle_no_conn() {}

    
    CommBufPtr            cbp;
    std::atomic<bool>     was_called;
    ConnQueue::Ptr        queue;
    const bool            insistent;
  };


  ConnQueue() : m_conn(nullptr), m_queue_running(false) { }

  virtual ~ConnQueue() { }

  bool connection(){
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_conn != nullptr && m_conn->is_open();
  }

  void stop() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    while(!m_queue.empty())
      m_queue.pop();
  
    if(m_conn != nullptr && m_conn->is_open())
      m_conn->do_close();
  }

  void put(ReqBase::Ptr req){
    if(req->queue == nullptr) 
      req->queue = shared_from_this();
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      m_queue.push(req);
    }
    exec_queue();
  }

  void set(client::ClientConPtr conn){
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      m_conn = conn;
    }
    exec_queue();
  }

  const std::string to_string() {
    std::string s("ConnQueue: ");
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    s.append("size=");
    s.append(std::to_string(m_queue.size()));
    s.append(" ");
    s.append(m_conn!=nullptr?client::to_string(m_conn):std::string("null"));
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
    ReqBase::Ptr req;

    for(;;) {
      {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if(m_queue.empty() || m_conn == nullptr || !m_conn->is_open()){
          m_queue_running = false;
          return;
        }
        req = m_queue.front();
      }
      
      HT_ASSERT(req->cbp != nullptr);

      if(!req->valid() 
        || m_conn->send_request(req->cbp, req) == Error::OK 
        || !req->insistent){

        if(!req->insistent)
          req->handle_no_conn();

        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_queue.pop();
        m_queue_running = !m_queue.empty();
        if(!m_queue_running) 
          return;
        continue;
      }
  
    }
  }

  std::recursive_mutex      m_mutex;
  std::queue<ReqBase::Ptr>  m_queue;
  client::ClientConPtr      m_conn;
  bool                      m_queue_running;
};


typedef std::shared_ptr<ConnQueue> ConnQueuePtr;
}} // server namespace


}
#endif // swc_lib_db_protocol_req_ConnQueue_h