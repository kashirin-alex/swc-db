/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/Checksum.h"



namespace SWC { 


ConnHandler::PendingRsp::PendingRsp(DispatchHandler::Ptr hdlr, 
                                    asio::high_resolution_timer* tm) 
                                    : hdlr(hdlr), tm(tm) {
}

ConnHandler::PendingRsp::~PendingRsp() { 
  if(tm) 
    delete tm; 
}


ConnHandler::Outgoing::Outgoing(CommBuf::Ptr cbuf, DispatchHandler::Ptr hdlr)
                                : cbuf(cbuf), hdlr(hdlr) { 
}

ConnHandler::Outgoing::~Outgoing() { }

ConnHandler::ConnHandler(AppContext::Ptr app_ctx) 
                        : app_ctx(app_ctx), m_next_req_id(0) {
}

ConnHandlerPtr ConnHandler::ptr() {
  return shared_from_this();
}

ConnHandler::~ConnHandler() { }


const std::string ConnHandler::endpoint_local_str() {
  std::string s(endpoint_local.address().to_string());
  s.append(":");
  s.append(std::to_string(endpoint_local.port()));
  return s;
}
  
const std::string ConnHandler::endpoint_remote_str() {
  std::string s(endpoint_remote.address().to_string());
  s.append(":");
  s.append(std::to_string(endpoint_remote.port()));
  return s;
}
  
const size_t ConnHandler::endpoint_remote_hash() {
  return endpoint_hash(endpoint_remote);
}
  
const size_t ConnHandler::endpoint_local_hash() {
  return endpoint_hash(endpoint_local);
}
  
void ConnHandler::new_connection() {
  auto ev = Event::make(Event::Type::ESTABLISHED, Error::OK);
  run(ev); 
}

const size_t ConnHandler::pending_read() {
  std::scoped_lock lock(m_mutex_reading);
  return m_pending.size();
}

const size_t ConnHandler::pending_write() {
  std::scoped_lock lock(m_mutex);
  return m_outgoing.size();
}

const bool ConnHandler::due() {
  return pending_read() || pending_write();
}

void ConnHandler::run(Event::Ptr& ev) {
  if(app_ctx != nullptr) 
    // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
    app_ctx->handle(ptr(), ev); 
}

void ConnHandler::do_close() {
  if(m_err == Error::OK) {
    auto ev = Event::make(Event::Type::DISCONNECT, m_err);
    run(ev);
  }
}

const int ConnHandler::send_error(int error, const std::string &msg, 
                                  const Event::Ptr& ev) {
  if(m_err != Error::OK)
    return m_err;

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

const int ConnHandler::response_ok(const Event::Ptr& ev) {
  if(m_err != Error::OK)
    return m_err;
      
  auto cbp = CommBuf::make(4);
  if(ev != nullptr)
    cbp->header.initialize_from_request_header(ev->header);
  cbp->append_i32(Error::OK);
  return send_response(cbp);
}

const int ConnHandler::send_response(CommBuf::Ptr &cbuf, 
                                     DispatchHandler::Ptr hdlr) {
  if(m_err != Error::OK)
    return m_err;

  cbuf->header.flags &= CommHeader::FLAGS_MASK_REQUEST;

  write_or_queue(new ConnHandler::Outgoing(cbuf, hdlr));
  return m_err;
}

const int ConnHandler::send_request(CommBuf::Ptr &cbuf, 
                                    DispatchHandler::Ptr hdlr) {
  if(m_err)
    return m_err;

  cbuf->header.flags |= CommHeader::FLAGS_BIT_REQUEST;
  //if(!cbuf->header.id) update id in-case cbuf switched over conns
  cbuf->header.id = next_req_id();

  write_or_queue(new ConnHandler::Outgoing(cbuf, hdlr));
  return m_err;
}

void ConnHandler::accept_requests() {
  m_accepting = true;
  read_pending();
}

/* 
void ConnHandler::accept_requests(DispatchHandler::Ptr hdlr, 
                                  uint32_t timeout_ms) {
  add_pending(new ConnHandler::PendingRsp(hdlr, get_timer(timeout_ms)));
  read_pending();
}
*/

const std::string ConnHandler::to_string() {
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

const uint32_t ConnHandler::next_req_id() {
  std::scoped_lock lock(m_mutex_reading);  
  while(m_pending.find(
    ++m_next_req_id == 0 ? ++m_next_req_id : m_next_req_id
    ) != m_pending.end()
  );
  return m_next_req_id;
}

asio::high_resolution_timer* ConnHandler::get_timer(const CommHeader& header) {
  if(!header.timeout_ms)
    return nullptr;

  auto tm = get_timer(header.timeout_ms); 

  auto ev = Event::make(Event::Type::ERROR, Error::REQUEST_TIMEOUT);
  ev->header.initialize_from_request_header(header);

  tm->async_wait(
    [ev, ptr=ptr()]
    (const asio::error_code ec) {
      if(ec != asio::error::operation_aborted)
        ptr->run_pending(ev);
    }
  );
  return tm;
}
    
void ConnHandler::write_or_queue(ConnHandler::Outgoing* data) { 
  {
    std::scoped_lock lock(m_mutex);  
    if(m_writing) {
      m_outgoing.push(data);
      return;
    }
    m_writing = true;
  }
  write(data);
}
  
void ConnHandler::next_outgoing() {
  ConnHandler::Outgoing* data;
  {
    std::scoped_lock lock(m_mutex);
    if(!(m_writing = !m_outgoing.empty())) 
      return;
    data = m_outgoing.front();
    m_outgoing.pop();
  }
  write(data); 
}
  
void ConnHandler::write(ConnHandler::Outgoing* data) {

  if(data->cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST) {
    std::scoped_lock lock(m_mutex_reading);
    m_pending.emplace(
      data->cbuf->header.id,
      new ConnHandler::PendingRsp(data->hdlr, get_timer(data->cbuf->header))
    );
  }
    
  std::vector<asio::const_buffer> buffers;
  data->cbuf->get(buffers);
  
  do_async_write(
    buffers,
    [data, ptr=ptr()]
    (const asio::error_code ec, uint32_t len) {
      delete data;
      if(ec) {
        ptr->do_close();
      } else {
        ptr->next_outgoing();
        ptr->read_pending();
      }
    }
  );
}

void ConnHandler::read_pending() {
  {
    std::scoped_lock lock(m_mutex_reading);
    if(m_err || m_reading)
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

  uint8_t* bufp;
  uint8_t* buf_header = bufp = new uint8_t[ev->header.header_len];
  for(uint8_t n=0; n<CommHeader::PREFIX_LENGTH; ++n)
    *bufp++ = *data++;

  do_async_read(
    bufp, 
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
    std::scoped_lock lock(m_mutex_reading);
    m_reading = false;
    more = m_accepting || !m_pending.empty();
  }
  if(more)
    read_pending();

  run_pending(ev);
}

void ConnHandler::disconnected() {
  ConnHandler::PendingRsp* pending;
  Event::Ptr ev;
  for(;;) {
    {
      std::scoped_lock lock(m_mutex_reading);
      if(m_pending.empty())
        return;
      pending = m_pending.begin()->second;
      m_pending.erase(m_pending.begin());
    }
    if(pending->tm != nullptr) 
      pending->tm->cancel();
    ev = Event::make(Event::Type::DISCONNECT, m_err);
    pending->hdlr->handle(ptr(), ev);
    delete pending;
  }
}

void ConnHandler::run_pending(Event::Ptr ev) {
  if(!ev->header.id) {
    run(ev);
    return;
  }

  ConnHandler::PendingRsp* pending = nullptr;
  {
    std::scoped_lock lock(m_mutex_reading);
    auto it = m_pending.find(ev->header.id);
    if(it != m_pending.end()) {
      pending = it->second;
      if(pending->tm != nullptr)
        pending->tm->cancel();
      m_pending.erase(it);
    }
  }

  if(pending) {
    pending->hdlr->handle(ptr(), ev);
    delete pending;
  } else {
    run(ev);
  }
}




ConnHandlerPlain::ConnHandlerPlain(AppContext::Ptr app_ctx, 
                                   SocketPlain& socket)
                                  : ConnHandler(app_ctx), 
                                    m_sock(std::move(socket)) {
}

ConnHandlerPlain::~ConnHandlerPlain() { 
  do_close();
}

void ConnHandlerPlain::do_close() {
  if(m_err == Error::OK) {
    close();
    ConnHandler::do_close();
  }
}

void ConnHandlerPlain::new_connection() {
  {
    std::scoped_lock lock(m_mutex);

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

const bool ConnHandlerPlain::is_open() {
  return m_err == Error::OK && m_sock.is_open();
}

void ConnHandlerPlain::close() {
  if(is_open()) {
    std::scoped_lock lock(m_mutex);
    try{m_sock.close();}catch(...){}
  }
  m_err = Error::COMM_NOT_CONNECTED;
  ConnHandler::disconnected();
}

asio::high_resolution_timer* ConnHandlerPlain::get_timer(uint32_t timeout_ms) {
  return new asio::high_resolution_timer(
    m_sock.get_executor(), std::chrono::milliseconds(timeout_ms));
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
  do_close();
}

void ConnHandlerSSL::do_close() {
  if(m_err == Error::OK) {
    close();
    ConnHandler::do_close();
  }
}

void ConnHandlerSSL::new_connection() {
  {
    std::scoped_lock lock(m_mutex);

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


const bool ConnHandlerSSL::is_open() {
  return m_err == Error::OK && m_sock.lowest_layer().is_open();
}

void ConnHandlerSSL::close() {
  if(is_open()) {
    std::scoped_lock lock(m_mutex);
    try{m_sock.lowest_layer().close();}catch(...){}
  }
  m_err = Error::COMM_NOT_CONNECTED;
  ConnHandler::disconnected();
}

asio::high_resolution_timer* ConnHandlerSSL::get_timer(uint32_t timeout_ms) {
  return new asio::high_resolution_timer(
    m_sock.get_executor(), std::chrono::milliseconds(timeout_ms));
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