/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/ClientConnQueue.h"

namespace SWC { namespace client {



ConnQueueReqBase::ConnQueueReqBase(bool insistent, const CommBuf::Ptr& cbp)
                                  : insistent(insistent), cbp(cbp), 
                                    was_called(false), queue(nullptr){
}

SWC_SHOULD_INLINE
ConnQueueReqBase::Ptr ConnQueueReqBase::req() {
  return std::dynamic_pointer_cast<ConnQueueReqBase>(shared_from_this());
}

ConnQueueReqBase::~ConnQueueReqBase() {}

void ConnQueueReqBase::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {
  if(was_called || !is_rsp(conn, ev))
    return;
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
}

bool ConnQueueReqBase::is_timeout(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
  if(out)
    request_again();
  return out;
}

bool ConnQueueReqBase::is_rsp(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
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
                    : m_ioctx(ioctx), m_conn(nullptr), 
                      m_connecting(false), m_qrunning(false),
                      m_timer(cfg_keepalive_ms
                        ? new asio::high_resolution_timer(*m_ioctx.get()) 
                        : nullptr
                      ),
                      cfg_keepalive_ms(keepalive_ms),
                      cfg_again_delay_ms(again_delay_ms) {
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
  if(m_timer)
    m_timer->cancel();
  for(;;) {
    Mutex::scope lock(m_mutex);
    auto it = m_delayed.begin();
    if(it == m_delayed.end()) {
      if(m_conn) {
        if(m_conn->is_open())
          m_conn->do_close();
        m_conn = nullptr;
      }
      break;
    }
    (*it)->cancel();
    m_delayed.erase(it);
    std::this_thread::yield();
  }
  for(;;) {
    Mutex::scope lock(m_mutex);
    if(!m_qrunning) {
      m_qrunning = true;
      break;
    }
    std::this_thread::yield();
  }
  for(ReqBase::Ptr req;;) {
    {
      Mutex::scope lock(m_mutex);
      if(empty()) {
        m_qrunning = false;
        break;
      }
      req = front();
      pop();
    }
    if(!req->was_called)
      req->handle_no_conn();
  }
}

void ConnQueue::put(const ConnQueue::ReqBase::Ptr& req) {
  if(!req->queue) 
    req->queue = shared_from_this();
  bool make_conn;
  {
    Mutex::scope lock(m_mutex);
    push(req);
    if((make_conn = (!m_conn || !m_conn->is_open()))) {
      if(m_connecting)
        return;
      m_connecting = true;
    }
  }
  if(make_conn && connect())
    return;
  exec_queue();
}

void ConnQueue::set(const ConnHandlerPtr& conn) {
  {
    Mutex::scope lock(m_mutex);
    m_conn = conn;
    m_connecting = false;
  }
  exec_queue();
}

void ConnQueue::delay(const ConnQueue::ReqBase::Ptr& req) {
  if(!cfg_again_delay_ms)
    return put(req);

  auto tm = new asio::high_resolution_timer(*m_ioctx.get());
  {
    Mutex::scope lock(m_mutex);
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
    Mutex::scope lock(m_mutex);
    m_delayed.erase(tm);
  }
  delete tm;
  put(req);
}

std::string ConnQueue::to_string() {
  std::string s("ConnQueue(size=");
  Mutex::scope lock(m_mutex);
  s.append(std::to_string(size()));
  s.append(" conn=");
  s.append(m_conn?m_conn->endpoint_remote_str():std::string("no"));
  s.append(")");
  return s;
}

void ConnQueue::exec_queue() {
  {
    Mutex::scope lock(m_mutex);
    if(empty() || m_qrunning)
      return;
    m_qrunning = true;
  }
  asio::post(*m_ioctx.get(), [ptr=shared_from_this()](){ptr->run_queue();});
}

void ConnQueue::run_queue() {
  if(m_timer)
    m_timer->cancel();

  ConnHandlerPtr conn;
  for(ReqBase::Ptr req;;) {
    {
      Mutex::scope lock(m_mutex);
      conn = (m_conn && m_conn->is_open()) ? m_conn : nullptr;
      req = front();
    }
    SWC_ASSERT(req->cbp);
    if(req->valid() && (!conn || !conn->send_request(req->cbp, req))) {
      req->handle_no_conn();
      if(req->insistent) {
        Mutex::scope lock(m_mutex);
        m_qrunning = false;
        break;
      }
    }
    {
      Mutex::scope lock(m_mutex);
      pop();
      if(empty()) {
        m_qrunning = false;
        break;
      }
    }
  }
  
  if(m_timer) // nullptr -eq persistent
    schedule_close();
    // ~ on timer after ms+ OR socket_opt ka(0)+interval(ms+) 
}

void ConnQueue::schedule_close() {
  m_timer->cancel();

  bool closing;
  ConnHandlerPtr conn;
  {
    Mutex::scope lock(m_mutex);
    if((closing = empty() && m_delayed.empty() &&
                 (!m_conn || !m_conn->due()))) {
      conn = m_conn;
      m_conn = nullptr;
    }
  }
  if(closing) {
    if(conn)
      conn->do_close();
    return close_issued();
  }

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