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
    
    CommBufPtr            cbp;
    std::atomic<bool>     was_called;
    ConnQueue::Ptr        queue;
    const bool            insistent;
  };


  ConnQueue() : m_conn(nullptr), 
                m_queue_running(false), 
                m_connecting(false) { }

  virtual ~ConnQueue() { }

  virtual bool connect() { 
    return false; // not implemented by default 
  }

  void stop() {
    
    std::cout << " ConnQueue stop\n";
    ReqBase::Ptr req;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if(m_conn != nullptr && m_conn->is_open())
      m_conn->do_close();

    while(!m_queue.empty()) {
      req = m_queue.front();
      m_queue.pop();
      if(!req->insistent && !req->was_called)
        req->handle_no_conn();
    }
  }

  void put(ReqBase::Ptr req){
    std::cout << " ConnQueue put " << req->to_string() << "\n";
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

  void set(client::ClientConPtr conn){
    std::cout << " ConnQueue set " << client::to_string(conn) << "\n";
    {
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      m_conn = conn;
      m_connecting = false;
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
    ReqBase::Ptr          req;
    client::ClientConPtr  conn;
    bool sent;
    for(;;) {
      {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if(m_queue.empty()){
          m_queue_running = false;
          return;
        } 
        req = m_queue.front();
        if((m_conn == nullptr || !m_conn->is_open()) && req->insistent) {
          m_queue_running = false;
          return;
        }
        conn = m_conn;
      }
      
      HT_ASSERT(req->cbp != nullptr);

      if(!req->valid() 
        || (sent = (conn != nullptr && conn->send_request(req->cbp, req) == Error::OK)) 
        || !req->insistent){

        if(!sent && !req->insistent)
          req->handle_no_conn();

        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_queue.pop();
        m_queue_running = !m_queue.empty();
        if(m_queue_running) 
          continue;
        return;
      }
  
    }
  }

  std::recursive_mutex      m_mutex;
  std::queue<ReqBase::Ptr>  m_queue;
  client::ClientConPtr      m_conn;
  bool                      m_queue_running;
  bool                      m_connecting;
};


typedef std::shared_ptr<ConnQueue> ConnQueuePtr;
}} // server namespace


}
#endif // swc_lib_db_protocol_req_ConnQueue_h