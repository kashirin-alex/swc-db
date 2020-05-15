/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/core/comm/ClientConnQueue.h"

namespace SWC { namespace client {



ConnQueueReqBase::ConnQueueReqBase(bool insistent, CommBuf::Ptr cbp)
                                  : insistent(insistent), cbp(cbp), 
                                    was_called(false), queue(nullptr){
}

ConnQueueReqBase::Ptr ConnQueueReqBase::req() {
  return std::dynamic_pointer_cast<ConnQueueReqBase>(shared_from_this());
}

ConnQueueReqBase::~ConnQueueReqBase() {}

void ConnQueueReqBase::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
  if(was_called || !is_rsp(conn, ev))
    return;
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
}

bool ConnQueueReqBase::is_timeout(ConnHandlerPtr conn, Event::Ptr& ev) {
  bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
  if(out)
    request_again();
  return out;
}

bool ConnQueueReqBase::is_rsp(ConnHandlerPtr conn, Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT 
     || ev->error == Error::Code::REQUEST_TIMEOUT) {
    if(!was_called)
      request_again();
    return false;
  }
  return true;
}

void ConnQueueReqBase::request_again() {
  if(!queue)
    run();
  else
    queue->delay(req());
}

bool ConnQueueReqBase::valid() { return true; }

void ConnQueueReqBase::handle_no_conn() {}

std::string ConnQueueReqBase::to_string() {
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
                      m_timer(cfg_keepalive_ms
                        ? new asio::high_resolution_timer(*m_ioctx.get()) 
                        : nullptr
                      ) {
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
      if(m_conn) {
        if(m_conn->is_open())
          m_conn->do_close();
        m_conn = nullptr;
      }
      if(m_timer)
        m_timer->cancel();
      break;
    }
    (*it)->cancel();
    m_delayed.erase(it);
    std::this_thread::yield();
  }
  while(!empty() && !activating()) 
    std::this_thread::yield();
  ReqBase::Ptr req;
  if(!empty()) do {
    if(!(req = front())->was_called)
      req->handle_no_conn();
  } while(!deactivating());
}

void ConnQueue::put(ConnQueue::ReqBase::Ptr req) {
  if(!req->queue) 
    req->queue = shared_from_this();
  push(req);
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(!m_conn || !m_conn->is_open())
      if(m_connecting)
        return;
      m_connecting = true;
      if(connect())
        return;
  }
  exec_queue();
}

void ConnQueue::set(const ConnHandlerPtr& conn) {
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_conn = conn;
    m_connecting = false;
  }
  exec_queue();
}

void ConnQueue::delay(ConnQueue::ReqBase::Ptr req) {
  if(!cfg_again_delay_ms)
    return put(req);

  auto tm = new asio::high_resolution_timer(*m_ioctx.get());
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_delayed.insert(tm);
  }
  tm->expires_from_now(std::chrono::milliseconds(cfg_again_delay_ms->get()));
  tm->async_wait(
    [req, tm](const asio::error_code ec) {
      req->queue->delay_proceed(req, tm);
    }
  );
}

void ConnQueue::delay_proceed(const ConnQueue::ReqBase::Ptr& req, 
                              asio::high_resolution_timer* tm) {
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_delayed.erase(tm);
  }
  delete tm;
  put(req);
}

std::string ConnQueue::to_string() {
  std::string s("ConnQueue:");
  s.append(" size=");
  s.append(std::to_string(size()));
  s.append(" conn=");
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    s.append(m_conn?m_conn->endpoint_remote_str():std::string("no"));
  }
  return s;
}

void ConnQueue::exec_queue() {
  if(activating())
    asio::post(*m_ioctx.get(), [ptr=shared_from_this()](){ptr->run_queue();});
}

void ConnQueue::run_queue() {
  if(m_timer) { 
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_timer->cancel();
  }
  ConnHandlerPtr  conn;
  do {
    ReqBase::Ptr& req = front();
    SWC_ASSERT(req->cbp);
    if(req->valid()) {
      {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        conn = (m_conn && m_conn->is_open()) ? m_conn : nullptr;
      }
      if(!conn || !conn->send_request(req->cbp, req)) {
        req->handle_no_conn();
        if(req->insistent) {
          deactivate();
          break;
        }
      }
    }
  } while(!deactivating());
  
  if(m_timer) // nullptr -eq persistent
    schedule_close();
    // ~ on timer after ms+ OR socket_opt ka(0)+interval(ms+) 
}

void ConnQueue::schedule_close() {
  if(empty()) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if(!m_conn || !m_conn->due()) {
      if(m_conn) {
        m_conn->do_close();
        m_conn = nullptr;
      }
      m_timer->cancel();
      return close_issued();
    }
  }
    
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  m_timer->cancel();
  m_timer->expires_from_now(
    std::chrono::milliseconds(cfg_keepalive_ms->get()));
  m_timer->async_wait(
    [ptr=shared_from_this()](const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted){
        ptr->schedule_close();
      }
    }
  );
}

}}