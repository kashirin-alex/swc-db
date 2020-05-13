/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/Checksum.h"



namespace SWC { 


ConnHandler::Pending::Pending(
          CommBuf::Ptr& cbuf, const DispatchHandler::Ptr& hdlr, 
          asio::high_resolution_timer* tm, const TimerCb_t& tm_cb)
          : cbuf(cbuf), hdlr(hdlr), tm(tm), tm_cb(tm_cb) {
}

ConnHandler::Pending::~Pending() {
  if(tm)
    delete tm;
}

void ConnHandler::Pending::arm_timer() {
  tm->expires_from_now(std::chrono::milliseconds(cbuf->header.timeout_ms));
  tm->async_wait(tm_cb);
  tm_cb = 0;
}


ConnHandler::ConnHandler(AppContext::Ptr app_ctx) 
                        : connected(false), 
                          app_ctx(app_ctx), m_next_req_id(0),
                          m_accepting(false), m_reading(false) {
}

ConnHandlerPtr ConnHandler::ptr() {
  return shared_from_this();
}

ConnHandler::~ConnHandler() { }


std::string ConnHandler::endpoint_local_str() {
  std::string s(endpoint_local.address().to_string());
  s.append(":");
  s.append(std::to_string(endpoint_local.port()));
  return s;
}
  
std::string ConnHandler::endpoint_remote_str() {
  std::string s(endpoint_remote.address().to_string());
  s.append(":");
  s.append(std::to_string(endpoint_remote.port()));
  return s;
}
  
size_t ConnHandler::endpoint_remote_hash() {
  return endpoint_hash(endpoint_remote);
}
  
size_t ConnHandler::endpoint_local_hash() {
  return endpoint_hash(endpoint_local);
}
  
void ConnHandler::new_connection() {
  connected = true;
  auto ev = Event::make(Event::Type::ESTABLISHED, Error::OK);
  run(ev); 
}

size_t ConnHandler::pending_read() {
  Mutex::scope lock(m_mutex);
  return m_pending.size();
}

size_t ConnHandler::pending_write() {
  return m_outgoing.size() + m_outgoing.is_active();
}

bool ConnHandler::due() {
  return m_outgoing.is_active() || m_outgoing.size() || pending_read();
}

void ConnHandler::run(Event::Ptr& ev) {
  if(app_ctx != nullptr) 
    // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
    app_ctx->handle(ptr(), ev); 
}

void ConnHandler::do_close() {
  auto ev = Event::make(Event::Type::DISCONNECT, Error::OK);
  run(ev);
}

bool ConnHandler::send_error(int error, const std::string& msg, 
                             const Event::Ptr& ev) {
  if(!connected)
    return false;

  size_t max_msg_size = std::numeric_limits<int16_t>::max();
  auto cbp = CommBuf::create_error_message(
    error, msg.length() < max_msg_size ? 
      msg.c_str() 
    :  msg.substr(0, max_msg_size).c_str()
  );
  if(ev != nullptr)
    cbp->header.initialize_from_request_header(ev->header);
  return send_response(cbp);
}

bool ConnHandler::response_ok(const Event::Ptr& ev) {
  if(!connected)
    return false;
      
  auto cbp = CommBuf::make(4);
  if(ev != nullptr)
    cbp->header.initialize_from_request_header(ev->header);
  cbp->append_i32(Error::OK);
  return send_response(cbp);
}

bool ConnHandler::send_response(CommBuf::Ptr& cbuf, 
                                DispatchHandler::Ptr hdlr) {
  if(!connected)
    return false;

  cbuf->header.flags &= CommHeader::FLAGS_MASK_REQUEST;
  write_or_queue(std::make_shared<Pending>(cbuf));
  return true;
}

bool ConnHandler::send_request(CommBuf::Ptr& cbuf, DispatchHandler::Ptr hdlr) {
  if(!connected)
    return false;
    
  auto& header = cbuf->header;
  header.flags |= CommHeader::FLAGS_BIT_REQUEST;

  auto pending = std::make_shared<Pending>(cbuf, hdlr,
    header.timeout_ms ? get_timer() : nullptr);

  assign_id:
    {
      Mutex::scope lock(m_mutex);
      if(header.id) {
        m_next_req_id = header.id;
        header.id = 0;
      } else {
        ++m_next_req_id;
      }
      if(!m_pending.emplace(m_next_req_id? m_next_req_id: ++m_next_req_id, 
                            pending).second)
        goto assign_id;
      header.id = m_next_req_id;
    }
  if(pending->tm) {
    auto ev = Event::make(Event::Type::ERROR, Error::REQUEST_TIMEOUT);
    ev->header.initialize_from_request_header(header);
    pending->tm_cb = [ev, conn=ptr()] (const asio::error_code ec) {
      if(ec != asio::error::operation_aborted) conn->run_pending(ev); };
  }
  write_or_queue(pending);
  return true;
}

void ConnHandler::accept_requests() {
  m_accepting = true;
  read_pending();
}

/* 
void ConnHandler::accept_requests(DispatchHandler::Ptr hdlr, 
                                  uint32_t timeout_ms) {
  add_pending(new ConnHandler::Pending(hdlr, get_timer(timeout_ms)));
  read_pending();
}
*/

std::string ConnHandler::to_string() {
  std::string s("Connection");

  if(!is_open()) {
    s.append(" CLOSED");
    return s;
  }

  s.append(" remote=");
  try{
    s.append(endpoint_remote_str());
    s.append(" (hash=");
    s.append(std::to_string(endpoint_remote_hash()));
    s.append(")");
  } catch(...){
    s.append("Exception");
  }
  s.append(" local=");
  try{
    s.append(endpoint_local_str());
    s.append(" (hash=");
    s.append(std::to_string(endpoint_local_hash()));
    s.append(")");
  } catch(...){
    s.append("Exception");
  }
  return s;
}

void ConnHandler::write_or_queue(ConnHandler::Pending::Ptr pending) {
  if(connected && m_outgoing.activating(pending))
    write(pending);
}

void ConnHandler::write(ConnHandler::Pending::Ptr& pending) {
  if(pending->tm) {
    Mutex::scope lock(m_mutex);
    pending->arm_timer();
  }
  do_async_write(
    pending->cbuf->get_buffers(),
    [pending, ptr=ptr()] (const asio::error_code ec, uint32_t len) { 
      ptr->write_complete(pending, ec);
    }
  );
}

void ConnHandler::write_complete(ConnHandler::Pending::Ptr pending, 
                                 const asio::error_code& ec) {
  if(ec || !connected)
    return do_close();
  
  if(pending->cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST)
    pending->cbuf = nullptr;
  //else if(pending->hdlr != nullptr) sent response event
  
  if(!m_outgoing.deactivating(&pending))
    write(pending);
  read_pending();
}

void ConnHandler::read_pending() {
  {
    Mutex::scope lock(m_mutex);
    if(!connected || m_reading)
      return;
    m_reading = true;
  }

  uint8_t* data = new uint8_t[CommHeader::PREFIX_LENGTH];

  do_async_read(
    data, 
    CommHeader::PREFIX_LENGTH,
    [data, ptr=ptr()] (const asio::error_code ec, size_t filled) {
      ptr->recved_header_pre(ec, data, filled);
      delete [] data;
    }
  );
}

void ConnHandler::recved_header_pre(asio::error_code ec,
                                    const uint8_t* data, size_t filled) {
  auto ev = Event::make(Event::Type::MESSAGE, Error::OK);

  if(filled != CommHeader::PREFIX_LENGTH) {
    ec = asio::error::eof;

  } else if(!ec) {
    try {
      const uint8_t* pre_bufp = data;
      ev->header.decode_prefix(&pre_bufp, &filled);
      if(!ev->header.header_len)
        throw;
    } catch(...) { 
      ec = asio::error::eof;
      ev->type = Event::Type::ERROR;
      ev->error = Error::REQUEST_TRUNCATED_HEADER;
      ev->header = CommHeader();
      SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_PREFIX_TRUNCATED: remain=%d", 
               filled);
    }
  }
  
  if(ec) {
    received(ev, ec);
    return;
  }

  uint8_t* buf_header;
  memcpy(buf_header = new uint8_t[ev->header.header_len], 
         data, CommHeader::PREFIX_LENGTH);
  do_async_read(
    buf_header + CommHeader::PREFIX_LENGTH, 
    ev->header.header_len - CommHeader::PREFIX_LENGTH,
    [ev, buf_header, ptr=ptr()](const asio::error_code ec, size_t filled) {
      ptr->recved_header(ev, ec, buf_header, filled);
      delete [] buf_header;
    }
  );
}

void ConnHandler::recved_header(const Event::Ptr& ev, asio::error_code ec,
                                const uint8_t* data, size_t filled) {
  if(filled + CommHeader::PREFIX_LENGTH != ev->header.header_len) {
    ec = asio::error::eof;
  
  } else if(!ec) {
    filled = ev->header.header_len;
    try {
      ev->header.decode(&data, &filled);
    } catch(...) {
      ec = asio::error::eof;
    }
  }

  if(ec) {
    ec = asio::error::eof;
    ev->type = Event::Type::ERROR;
    ev->error = Error::REQUEST_TRUNCATED_HEADER;
    ev->header = CommHeader();
    SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_TRUNCATED: len=%d", ev->header.header_len);
  }

  if(ec || !ev->header.buffers)
    received(ev, ec);
  else
    recv_buffers(ev, 0);
}

void ConnHandler::recv_buffers(const Event::Ptr& ev, uint8_t n) {

  StaticBuffer* buffer;
  size_t remain;
  if(n == 0) {
    buffer = &ev->data;
    remain = ev->header.data_size;
  } else { 
    buffer = &ev->data_ext;
    remain = ev->header.data_ext_size;
  }
  buffer->reallocate(remain);

  do_async_read(
    buffer->base, 
    remain,
    [ev, n, ptr=ptr()](const asio::error_code ec, size_t filled) {
      ptr->recved_buffer(ev, ec, n, filled);
    }
  );
}

void ConnHandler::recved_buffer(const Event::Ptr& ev, asio::error_code ec,
                                uint8_t n, size_t filled) {
  if(!ec) {
    StaticBuffer* buffer;
    uint32_t checksum;
    if(n == 0) {
      buffer = &ev->data;
      checksum = ev->header.data_chksum;
    } else { 
      buffer = &ev->data_ext;
      checksum = ev->header.data_ext_chksum;
    }
  
    if(filled != buffer->size || 
       !checksum_i32_chk(checksum, buffer->base, buffer->size)) {
      ec = asio::error::eof;
      ev->error = Error::REQUEST_TRUNCATED_PAYLOAD;
      ev->data.free();
      ev->data_ext.free();
      SWC_LOGF(LOG_WARN, "read, REQUEST PAYLOAD_TRUNCATED: n(%d) error=(%s) %s", 
               (int16_t)n, ec.message().c_str(), ev->to_str().c_str());
    }
  }
  
  if(ec || ev->header.buffers == ++n)
    received(ev, ec);
  else
    recv_buffers(ev, n);
}

void ConnHandler::received(const Event::Ptr& ev, const asio::error_code& ec) {
  if(ec) {
    do_close();
    return;
  }

  if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
    ev->received();

  bool more;
  {
    Mutex::scope lock(m_mutex);
    m_reading = false;
    more = m_accepting || !m_pending.empty();
  }
  if(more)
    read_pending();

  run_pending(ev);
}

void ConnHandler::disconnected() {
  Event::Ptr ev = Event::make(Event::Type::DISCONNECT, Error::OK);
  Pending::Ptr pending;
  while(!m_outgoing.deactivating(&pending)) {
    if(pending->cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      continue;
     if(pending->tm) {
      Mutex::scope lock(m_mutex);
      pending->tm->cancel();
    }
    if(pending->hdlr != nullptr)
      pending->hdlr->handle(ptr(), ev);
  }
  for(;;) {
    {
      Mutex::scope lock(m_mutex);
      if(m_pending.empty())
        return;
      if((pending = m_pending.begin()->second)->tm)
        pending->tm->cancel();
      m_pending.erase(m_pending.begin());
    }
    pending->hdlr->handle(ptr(), ev);
  }
}

void ConnHandler::run_pending(Event::Ptr ev) {
  if(!ev->header.id)
    return run(ev);

  Pending::Ptr pending;
  {
    Mutex::scope lock(m_mutex);
    auto it = m_pending.find(ev->header.id);
    if(it == m_pending.end()) {
      pending = nullptr;
    } else {
      if((pending = it->second)->tm)
        pending->tm->cancel();
      m_pending.erase(it);
    }
  }
  pending == nullptr 
    ? run(ev)
    : pending->hdlr->handle(ptr(), ev);
}




ConnHandlerPlain::ConnHandlerPlain(AppContext::Ptr app_ctx, 
                                   SocketPlain& socket)
                                  : ConnHandler(app_ctx), 
                                    m_sock(std::move(socket)) {
}

ConnHandlerPlain::~ConnHandlerPlain() {
  if(connected && m_sock.is_open())
    try{ m_sock.close(); } catch(...) { }
}

void ConnHandlerPlain::do_close() {
  if(connected) {
    close();
    ConnHandler::do_close();
  }
}

void ConnHandlerPlain::close() {
  bool once;
  {
    Mutex::scope lock(m_mutex);
    if(once = connected)
      connected = false;
  }
  if(once) {
    if(m_sock.is_open())
      try{ m_sock.close(); } catch(...) { }
    disconnected();
  }
}

void ConnHandlerPlain::new_connection() {
  {
    Mutex::scope lock(m_mutex);

    endpoint_remote = m_sock.remote_endpoint();
    endpoint_local = m_sock.local_endpoint();
    SWC_LOGF(
      LOG_DEBUG, 
      "new_connection local=%s, remote=%s, executor=%d",
      endpoint_local_str().c_str(), endpoint_remote_str().c_str(),
      (size_t)&m_sock.get_executor().context());
  }
  ConnHandler::new_connection();
}

bool ConnHandlerPlain::is_open() {
  return connected && m_sock.is_open();
}

asio::high_resolution_timer* ConnHandlerPlain::get_timer() {
  return new asio::high_resolution_timer(m_sock.get_executor());
}

void ConnHandlerPlain::do_async_write(
        const std::vector<asio::const_buffer>& buffers,
        const std::function<void(const asio::error_code, uint32_t)>& hdlr) {
  asio::async_write(m_sock, buffers, hdlr);
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        const std::function<void(const asio::error_code, size_t)>& hdlr) {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), hdlr);
}

void ConnHandlerPlain::read(uint8_t** bufp, size_t* remainp, 
                            asio::error_code &ec) {
  size_t read = 0;
  do {
    read = m_sock.read_some(asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}




ConnHandlerSSL::ConnHandlerSSL(AppContext::Ptr app_ctx, 
                               asio::ssl::context& ssl_ctx,
                               SocketPlain& socket)
                              : ConnHandler(app_ctx), 
                                m_sock(std::move(socket), ssl_ctx) {
}

ConnHandlerSSL::~ConnHandlerSSL() { 
  if(connected && m_sock.lowest_layer().is_open())
    try{ m_sock.lowest_layer().close(); } catch(...) { }
}

void ConnHandlerSSL::do_close() {
  if(connected) {
    close();
    ConnHandler::do_close();
  }
}

void ConnHandlerSSL::close() {
  bool once;
  {
    Mutex::scope lock(m_mutex);
    if(once = connected)
      connected = false;
  }
  if(once) {
    if(m_sock.lowest_layer().is_open())
      try{ m_sock.lowest_layer().close(); } catch(...) { }
    disconnected();
  }
}

void ConnHandlerSSL::new_connection() {
  {
    Mutex::scope lock(m_mutex);

    endpoint_remote = m_sock.lowest_layer().remote_endpoint();
    endpoint_local = m_sock.lowest_layer().local_endpoint();
    SWC_LOGF(
      LOG_DEBUG, 
      "new_connection local=%s, remote=%s, executor=%d",
      endpoint_local_str().c_str(), endpoint_remote_str().c_str(),
      (size_t)&m_sock.get_executor().context());
  }
  ConnHandler::new_connection();
}

bool ConnHandlerSSL::is_open() {
  return connected && m_sock.lowest_layer().is_open();
}

void ConnHandlerSSL::handshake() {
  m_sock.async_handshake(
    asio::ssl::stream_base::server,
    [ptr=ptr()](const asio::error_code ec) {
      if(!ec) {
        ptr->accept_requests();
      } else {
        SWC_LOGF(LOG_DEBUG, "handshake error=%d(%s)", 
                ec.value(), ec.message().c_str());
        ptr->do_close();
      }
    }
  );
}

void ConnHandlerSSL::set_verify(
    const std::function<bool(bool, asio::ssl::verify_context&)>& cb) {
  m_sock.set_verify_callback(cb);
}

void ConnHandlerSSL::handshake_client(
            const std::function<void(const asio::error_code&)> cb) {
  m_sock.async_handshake(asio::ssl::stream_base::client, cb);
}

void ConnHandlerSSL::handshake_client(asio::error_code& ec) {
  m_sock.handshake(asio::ssl::stream_base::client, ec);
}


asio::high_resolution_timer* ConnHandlerSSL::get_timer() {
  return new asio::high_resolution_timer(m_sock.get_executor());
}
  
void ConnHandlerSSL::do_async_write(
        const std::vector<asio::const_buffer>& buffers,
        const std::function<void(const asio::error_code, uint32_t)>& hdlr) {
  asio::async_write(m_sock, buffers, hdlr);
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        const std::function<void(const asio::error_code, size_t)>& hdlr) {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), hdlr);
}

void ConnHandlerSSL::read(uint8_t** bufp, size_t* remainp, 
                            asio::error_code &ec) {
  size_t read = 0;
  do {
    read = m_sock.read_some(asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}
 

}