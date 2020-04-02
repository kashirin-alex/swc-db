/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/core/comm/ClientConnQueue.h"

namespace SWC { namespace client {



ConnQueue::ReqBase::ReqBase(bool insistent, CommBuf::Ptr cbp)
                            : insistent(insistent), cbp(cbp), 
                              was_called(false), queue(nullptr){
}

ConnQueue::ReqBase::Ptr ConnQueue::ReqBase::req() {
  return std::dynamic_pointer_cast<ReqBase>(shared_from_this());
}

ConnQueue::ReqBase::~ReqBase() {}

void ConnQueue::ReqBase::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
  if(was_called || !is_rsp(conn, ev))
    return;
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
}

bool ConnQueue::ReqBase::is_timeout(ConnHandlerPtr conn, Event::Ptr& ev) {
  bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
  if(out)
    request_again();
  return out;
}

bool ConnQueue::ReqBase::is_rsp(ConnHandlerPtr conn, Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT 
     || ev->error == Error::Code::REQUEST_TIMEOUT) {
    if(!was_called)
      request_again();
    return false;
  }
  return true;
}

void ConnQueue::ReqBase::request_again() {
  if(queue == nullptr)
    run();
  else
    queue->delay(req());
}

bool ConnQueue::ReqBase::valid() { return true; }

void ConnQueue::ReqBase::handle_no_conn() {}

std::string ConnQueue::ReqBase::to_string() {
  std::string s("ReqBase(");
  s.append(" called=");
  s.append(std::to_string(was_called.load()));
  s.append(" insistent=");
  s.append(std::to_string(insistent));
  s.append(" ");
  s.append(cbp->header.to_string());
  s.append(")");
  return s;
}


ConnQueue::ConnQueue(IOCtxPtr ioctx,
                     const Property::V_GINT32::Ptr keepalive_ms, 
                     const Property::V_GINT32::Ptr again_delay_ms) 
                    : m_ioctx(ioctx), m_conn(nullptr), m_connecting(false),
                      cfg_keepalive_ms(keepalive_ms),
                      cfg_again_delay_ms(again_delay_ms), 
                      m_timer(nullptr) { 
}

ConnQueue::~ConnQueue() {
  if(m_timer)
    delete m_timer;
}

bool ConnQueue::connect() { 
  return false; // not implemented by default 
}

void ConnQueue::close_issued() { }

void ConnQueue::stop() {
  for(;;) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_delayed.begin();
    if(it == m_delayed.end()) {
      break;
    } else {
      (*it)->cancel();
      std::this_thread::yield();
    }
  }
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(m_conn != nullptr && m_conn->is_open())
      m_conn->do_close();

    if(m_timer != nullptr) {
      m_timer->cancel();
      m_timer = nullptr;
    }
  }
  ReqBase::Ptr req;
  while(!m_queue.empty() && !m_queue.activating()) 
    std::this_thread::yield();
  if(!m_queue.empty()) do {
    if(!(req = m_queue.front())->was_called)
      req->handle_no_conn();
  } while(!m_queue.deactivating());
}

void ConnQueue::put(ConnQueue::ReqBase::Ptr req) {
  if(req->queue == nullptr) 
    req->queue = shared_from_this();
  m_queue.push(req);
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(m_conn == nullptr || !m_conn->is_open())
      if(m_connecting)
        return;
      m_connecting = true;
      if(connect())
        return;
  }
  exec_queue();
}

void ConnQueue::set(ConnHandlerPtr conn) {
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_conn = conn;
    m_connecting = false;
  }
  exec_queue();
}

void ConnQueue::delay(ConnQueue::ReqBase::Ptr req) {
  if(cfg_again_delay_ms == nullptr) {
    put(req);
    return;
  }
  auto tm = new asio::high_resolution_timer(*m_ioctx.get());
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_delayed.push_back(tm);
  }
  tm->expires_from_now(std::chrono::milliseconds(cfg_again_delay_ms->get()));
  tm->async_wait(
    [tm, req, queue=shared_from_this()](const asio::error_code ec) {
      queue->delay_proceed(req, tm);
    }
  );
}

void ConnQueue::delay_proceed(ConnQueue::ReqBase::Ptr req, 
                              asio::high_resolution_timer* tm) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  auto it = std::find(m_delayed.begin(), m_delayed.end(), tm);
  if(it != m_delayed.end()) {
    m_delayed.erase(it);
    delete tm;
    put(req);
  }
}

std::string ConnQueue::to_string() {
  std::string s("ConnQueue:");
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  s.append(" size=");
  s.append(std::to_string(m_queue.size()));
  s.append(" conn=");
  s.append(m_conn!=nullptr?m_conn->endpoint_remote_str():std::string("no"));
  return s;
}

void ConnQueue::exec_queue() {
  if(m_queue.activating())
    asio::post(*m_ioctx.get(), [ptr=shared_from_this()](){ptr->run_queue();});
}

void ConnQueue::run_queue() {
  { 
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(m_timer != nullptr) {
      m_timer->cancel();
      m_timer = nullptr;
    }
  }
  ReqBase::Ptr    req;
  ConnHandlerPtr  conn;
  do {
    SWC_ASSERT((req = m_queue.front())->cbp != nullptr);
    if(req->valid()) {
      {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        conn = (m_conn != nullptr && m_conn->is_open()) ? m_conn : nullptr;
      }
      if(conn == nullptr || !conn->send_request(req->cbp, req)) {
        req->handle_no_conn();
        if(req->insistent) {
          m_queue.deactivate();
          break;
        }
      }
    }
  } while(!m_queue.deactivating());
  
  schedule_close();
}

void ConnQueue::schedule_close() {
  if(cfg_keepalive_ms == nullptr) // nullptr -eq persistent
    return;
  // ~ on timer after ms+ OR socket_opt ka(0)+interval(ms+) 

  if(m_queue.empty()) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(m_conn == nullptr || !m_conn->due()) {
      if(m_conn != nullptr)
        m_conn->do_close(); 
      if(m_timer != nullptr) {
        m_timer->cancel();
        m_timer = nullptr;
      }
      close_issued();
      return;
    }
  }
    
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  if(m_timer == nullptr)
    m_timer = new asio::high_resolution_timer(*m_ioctx.get());
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

}}