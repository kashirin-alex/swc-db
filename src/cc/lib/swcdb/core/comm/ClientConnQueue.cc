/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/ClientConnQueue.h"

namespace SWC { namespace Comm { namespace client {



ConnQueueReqBase::ConnQueueReqBase(bool insistent, const Buffers::Ptr& cbp)
                                  noexcept
                                  : insistent(insistent), cbp(cbp),
                                    queue(nullptr) {
}

SWC_SHOULD_INLINE
ConnQueueReqBase::Ptr ConnQueueReqBase::req() noexcept {
  return std::dynamic_pointer_cast<ConnQueueReqBase>(shared_from_this());
}

ConnQueueReqBase::~ConnQueueReqBase() {}

void ConnQueueReqBase::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(!is_rsp(ev))
    return;
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
}

bool ConnQueueReqBase::is_timeout(const Event::Ptr& ev) {
  bool out = ev->error == Error::Code::REQUEST_TIMEOUT;
  if(out)
    request_again();
  return out;
}

bool ConnQueueReqBase::is_rsp(const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT ||
     ev->error == Error::Code::REQUEST_TIMEOUT) {
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

void ConnQueueReqBase::handle_no_conn() { }

void ConnQueueReqBase::print(std::ostream& out) {
  cbp->header.print(out << "ReqBase(insistent=" << insistent << ' ');
  out << ')';
}


ConnQueue::ConnQueue(const IoContextPtr& ioctx,
                     const Config::Property::V_GINT32::Ptr keepalive_ms,
                     const Config::Property::V_GINT32::Ptr again_delay_ms)
                    : cfg_keepalive_ms(keepalive_ms),
                      cfg_again_delay_ms(again_delay_ms),
                      m_ioctx(ioctx), m_conn(nullptr),
                      m_connecting(false),
                      m_timer(cfg_keepalive_ms
                        ? new asio::high_resolution_timer(m_ioctx->executor())
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
  if(m_timer)
    m_timer->cancel();
  for(;;) {
    Core::MutexSptd::scope lock(m_mutex);
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

  while(m_q_state.running())
    std::this_thread::yield();

  for(ReqBase::Ptr req;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(empty()) {
        m_q_state.stop();
        break;
      }
      req = front();
      pop();
    }
    req->handle_no_conn();
  }
}

EndPoint ConnQueue::get_endpoint_remote() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_conn && m_conn->is_open() ? m_conn->endpoint_remote : EndPoint();
}

EndPoint ConnQueue::get_endpoint_local() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_conn && m_conn->is_open() ? m_conn->endpoint_local : EndPoint();
}

void ConnQueue::put(const ConnQueue::ReqBase::Ptr& req) {
  if(!req->queue)
    req->queue = shared_from_this();
  bool make_conn;
  {
    Core::MutexSptd::scope lock(m_mutex);
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
    Core::MutexSptd::scope lock(m_mutex);
    m_conn = conn;
    m_connecting = false;
  }
  exec_queue();
}

void ConnQueue::delay(const ConnQueue::ReqBase::Ptr& req) {
  if(!cfg_again_delay_ms)
    return put(req);

  auto tm = new asio::high_resolution_timer(m_ioctx->executor());
  tm->expires_after(std::chrono::milliseconds(cfg_again_delay_ms->get()));

  Core::MutexSptd::scope lock(m_mutex);
  m_delayed.insert(tm);
  tm->async_wait([this, req, tm](const asio::error_code& ec) {
    if(ec == asio::error::operation_aborted) {
      req->handle_no_conn();
    } else {
      {
        Core::MutexSptd::scope lock(m_mutex);
        m_delayed.erase(tm);
      }
      put(req);
    }
    delete tm;
  });
}

void ConnQueue::print(std::ostream& out) {
  out << "ConnQueue(size=";
  Core::MutexSptd::scope lock(m_mutex);
  out << size() << " conn=";
  if(m_conn)
    m_conn->print(out);
  else
    out << "no";
  out << ')';
}

void ConnQueue::exec_queue() {
  if(m_q_state.running())
    return;
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(empty()) {
      m_q_state.stop();
      return;
    }
  }
  m_ioctx->post([ptr=shared_from_this()](){ptr->run_queue();});
}

void ConnQueue::run_queue() {
  if(m_timer)
    m_timer->cancel();

  ConnHandlerPtr conn;
  for(ReqBase::Ptr req;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      conn = (m_conn && m_conn->is_open()) ? m_conn : nullptr;
      req = front();
    }
    SWC_ASSERT(req->cbp);
    if(req->valid() && (!conn || !conn->send_request(req->cbp, req))) {
      if(req->insistent) {
        m_q_state.stop();
        req->handle_no_conn();
        break;
      }
      req->handle_no_conn();
    }
    {
      Core::MutexSptd::scope lock(m_mutex);
      pop();
      if(empty()) {
        m_q_state.stop();
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
    Core::MutexSptd::scope lock(m_mutex);
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

  m_timer->expires_after(std::chrono::milliseconds(cfg_keepalive_ms->get()));
  m_timer->async_wait(
    [ptr=shared_from_this()](const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted){
        ptr->schedule_close();
      }
    }
  );
}

}}}
