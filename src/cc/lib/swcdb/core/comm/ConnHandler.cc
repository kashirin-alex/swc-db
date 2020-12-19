/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/Checksum.h"



namespace SWC { namespace Comm {


SWC_SHOULD_INLINE
ConnHandler::Pending::Pending(const Buffers::Ptr& cbuf, 
                              DispatchHandler::Ptr& hdlr)
                              : cbuf(cbuf), hdlr(hdlr), timer(nullptr) {
}

ConnHandler::Pending::~Pending() {
  if(timer)
    delete timer;
}



SWC_SHOULD_INLINE
ConnHandler::ConnHandler(AppContext::Ptr& app_ctx) 
                        : connected(true), 
                          app_ctx(app_ctx), m_next_req_id(0),
                          m_accepting(false) {
}

SWC_SHOULD_INLINE
ConnHandlerPtr ConnHandler::ptr() {
  return shared_from_this();
}

ConnHandler::~ConnHandler() { 
  Pending* pending;
  while(!m_outgoing.deactivating(&pending))
    delete pending;

  for(auto it=m_pending.begin(); it!=m_pending.end(); ++it)
    delete it->second;
}

SWC_SHOULD_INLINE
size_t ConnHandler::endpoint_remote_hash() const {
  return endpoint_hash(endpoint_remote);
}
  
SWC_SHOULD_INLINE
size_t ConnHandler::endpoint_local_hash() const {
  return endpoint_hash(endpoint_local);
}

SWC_SHOULD_INLINE
Core::Encoder::Type ConnHandler::get_encoder() const {
  return !app_ctx->cfg_encoder || 
         endpoint_local.address() == endpoint_remote.address()
          ? Core::Encoder::Type::PLAIN
          : Core::Encoder::Type(app_ctx->cfg_encoder->get());
}
  
void ConnHandler::new_connection() {
  auto sock = socket_layer();
  {
    Core::MutexSptd::scope lock(m_mutex);
    endpoint_remote = sock->remote_endpoint();
    endpoint_local = sock->local_endpoint();
  }
  SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "New-"); );
  run(Event::make(Event::Type::ESTABLISHED, Error::OK)); 
}

size_t ConnHandler::pending_read() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_pending.size();
}

SWC_SHOULD_INLINE
size_t ConnHandler::pending_write() {
  return m_outgoing.size() + m_outgoing.is_active();
}

SWC_SHOULD_INLINE
bool ConnHandler::due() {
  return m_outgoing.is_active() || m_outgoing.size() || pending_read();
}

SWC_SHOULD_INLINE
void ConnHandler::run(const Event::Ptr& ev) {
  //if(ev->header.flags & Header::FLAGS_BIT_REQUEST)
  app_ctx->handle(ptr(), ev);
}

void ConnHandler::do_close() {
  run(Event::make(Event::Type::DISCONNECT, Error::OK));
}

bool ConnHandler::send_error(int error, const std::string& msg, 
                             const Event::Ptr& ev) noexcept {
  return send_response(
    Buffers::create_error_message(
      ev,
      error, msg.c_str(), msg.length() < INT16_MAX? msg.length() : INT16_MAX)
    );
}

bool ConnHandler::response_ok(const Event::Ptr& ev) noexcept {
  auto cbp = Buffers::make(ev, 4);
  cbp->append_i32(Error::OK);
  return send_response(cbp);
}

bool ConnHandler::send_response(const Buffers::Ptr& cbuf,
                                DispatchHandler::Ptr hdlr) noexcept {
  if(!connected || cbuf->expired())
    return false;
  cbuf->header.flags &= Header::FLAGS_MASK_REQUEST;
  write_or_queue(new Pending(cbuf, hdlr));
  return true;
}

bool ConnHandler::send_request(Buffers::Ptr& cbuf, 
                               DispatchHandler::Ptr hdlr) {
  if(!connected)
    return false;
  cbuf->header.flags |= Header::FLAGS_BIT_REQUEST;
  write_or_queue(new Pending(cbuf, hdlr));
  return true;
}

void ConnHandler::accept_requests() {
  m_accepting.store(true, std::memory_order_relaxed);
  read_pending();
}

/* 
void ConnHandler::accept_requests(DispatchHandler::Ptr hdlr, 
                                  uint32_t timeout_ms) {
  add_pending(new ConnHandler::Pending(hdlr, get_timer(timeout_ms)));
  read_pending();
}
*/

void ConnHandler::print(std::ostream& out) const {
  out << "Connection(encoder=" << Core::Encoder::to_string(get_encoder());
  if(is_open()) {
    out << " remote=" << endpoint_remote
        << " local=" << endpoint_local
        << ' ' << (is_secure() ? "SECURE" : "PLAIN");
  } else {
    out << " CLOSED";
  }
  out << ')';
}

void ConnHandler::write_or_queue(ConnHandler::Pending* pending) {
  pending->cbuf->prepare(get_encoder());

  if(m_outgoing.activating(pending))
    write(pending);
}

void ConnHandler::write_next() {
  Pending* pending;
  if(!m_outgoing.deactivating(&pending))
    write(pending);
  read_pending();
}

void ConnHandler::write(ConnHandler::Pending* pending) {
  auto cbuf = std::move(pending->cbuf);
  auto& header = cbuf->header;

  if(!pending->hdlr || header.flags & Header::FLAGS_BIT_IGNORE_RESPONSE) {
    // send request/response without sent/rsp-ack
    delete pending;
    if(cbuf->expired())
      return write_next();
    goto write_commbuf;
  }

  if(!(header.flags & Header::FLAGS_BIT_REQUEST)) {
    //if(!header.timeout_ms) {
      // send_response with sent-ack
      auto hdlr = std::move(pending->hdlr);
      delete pending;
      if(cbuf->expired())
        return write_next();
    
      do_async_write(
        cbuf->get_buffers(),
        [cbuf, hdlr, conn=ptr()] (const asio::error_code& ec, uint32_t) { 
          if(!ec)
            conn->write_next();

          auto ev = Event::make(
            Event::Type::ERROR, ec ? Error::COMM_SEND_ERROR : Error::OK);
          ev->header.initialize_from_response(cbuf->header);
          hdlr->handle(conn, ev);
          if(ec)
            return conn->do_close();
        }
      );
    //} else {
      // send_response expect sent/timeout ack with m_sends
    return;
  }
  
  // send_request expect rsp/timeout with m_pending
  if(header.timeout_ms)
    pending->timer = new asio::high_resolution_timer(
      socket_layer()->get_executor());

  assign_id: {
    Core::MutexSptd::scope lock(m_mutex);
    if(header.id) {
      m_next_req_id = header.id;
      header.id = 0;
    } else if(!++m_next_req_id) {
      ++m_next_req_id;
    }
    if(!m_pending.emplace(m_next_req_id, pending).second)
      goto assign_id;
    header.id = m_next_req_id;
  
    if(pending->timer) {
      pending->timer->expires_after(
        std::chrono::milliseconds(header.timeout_ms));
      pending->timer->async_wait(
        [cbuf, conn=ptr()] (const asio::error_code& ec) {
          if(ec == asio::error::operation_aborted) 
            return;
          auto ev = Event::make(Event::Type::ERROR, Error::REQUEST_TIMEOUT);
          ev->header.initialize_from_request(cbuf->header);
          conn->run_pending(ev); 
        }
      );
    }
  }

  write_commbuf:
    do_async_write(
      cbuf->get_buffers(),
      [cbuf, conn=ptr()] (const asio::error_code& ec, uint32_t) {
        if(ec) {
          conn->do_close();
        } else {
          conn->write_next();
        }
      }
    );
}

void ConnHandler::read_pending() {
  if(!connected || m_read.running())
    return;

  uint8_t* data = new uint8_t[Header::PREFIX_LENGTH];

  do_async_read(
    data, 
    Header::PREFIX_LENGTH,
    [data, ptr=ptr()] (const asio::error_code& ec, size_t filled) {
      ptr->recved_header_pre(ec, data, filled);
      delete [] data;
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_header_pre(asio::error_code ec,
                                    const uint8_t* data, size_t filled) {
  auto ev = Event::make(Event::Type::MESSAGE, Error::OK);

  if(filled != Header::PREFIX_LENGTH) {
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
      ev->header.reset();
      SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_PREFIX_TRUNCATED: remain=%lu", 
               filled);
    }
  }
  
  if(ec) {
    received(ev, ec);
    return;
  }

  uint8_t* buf_header;
  memcpy(buf_header = new uint8_t[ev->header.header_len], 
         data, Header::PREFIX_LENGTH);
  do_async_read(
    buf_header + Header::PREFIX_LENGTH, 
    ev->header.header_len - Header::PREFIX_LENGTH,
    [ev, buf_header, ptr=ptr()](const asio::error_code& ec, size_t filled) {
      ptr->recved_header(ev, ec, buf_header, filled);
      delete [] buf_header;
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_header(const Event::Ptr& ev, asio::error_code ec,
                                const uint8_t* data, size_t filled) {
  if(filled + Header::PREFIX_LENGTH != ev->header.header_len) {
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
    ev->header.reset();
    SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_TRUNCATED: len=%d", 
             ev->header.header_len);
  }

  if(ec || !ev->header.buffers)
    received(ev, ec);
  else
    recv_buffers(ev, 0);
}

void ConnHandler::recv_buffers(const Event::Ptr& ev, uint8_t n) {

  StaticBuffer* buffer;
  size_t remain;
  if(!n) {
    buffer = &ev->data;
    remain = ev->header.data.size;
  } else {
    buffer = &ev->data_ext;
    remain = ev->header.data_ext.size;
  }
  buffer->reallocate(remain);

  do_async_read(
    buffer->base, 
    remain,
    [ev, n, ptr=ptr()](const asio::error_code& ec, size_t filled) {
      ptr->recved_buffer(ev, ec, n, filled);
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_buffer(const Event::Ptr& ev, asio::error_code ec,
                                uint8_t n, size_t filled) {
  if(!ec) {
    StaticBuffer* buffer;
    uint32_t checksum;
    if(!n) {
      buffer = &ev->data;
      checksum = ev->header.data.chksum;
    } else { 
      buffer = &ev->data_ext;
      checksum = ev->header.data_ext.chksum;
    }
  
    if(filled != buffer->size || 
       !Core::checksum_i32_chk(checksum, buffer->base, buffer->size)) {
      ec = asio::error::eof;
      ev->type = Event::Type::ERROR;
      ev->error = Error::REQUEST_TRUNCATED_PAYLOAD;
      ev->data.free();
      ev->data_ext.free();
      SWC_LOG_OUT(LOG_WARN,
        SWC_LOG_OSTREAM 
          << "read, REQUEST PAYLOAD_TRUNCATED: n(" << int(n) << ") ";
        ev->print(SWC_LOG_OSTREAM);
      );
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

  if(ev->header.flags & Header::FLAGS_BIT_REQUEST)
    ev->received();

  bool more = m_accepting.load(std::memory_order_relaxed);
  if(!more) {
    Core::MutexSptd::scope lock(m_mutex);
    more = !m_pending.empty();
  }
  m_read.stop();
  if(more)
    read_pending();

  run_pending(ev);
}

void ConnHandler::disconnected() {
  auto ev = Event::make(Event::Type::DISCONNECT, Error::COMM_NOT_CONNECTED);
  Pending* pending;
  while(!m_outgoing.deactivating(&pending)) {
    if(pending->hdlr)
      pending->hdlr->handle(ptr(), ev);
    delete pending;
  }
  for(;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_pending.empty())
        return;
      pending = m_pending.begin()->second;
      m_pending.erase(m_pending.begin());
    }
    if(pending->timer)
      pending->timer->cancel();
    pending->hdlr->handle(ptr(), ev);
    delete pending;
  }
}

void ConnHandler::run_pending(const Event::Ptr& ev) {
  Pending* pending = nullptr;
  if(ev->header.id) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = m_pending.find(ev->header.id);
      if(it != m_pending.end()) {
        pending = it->second;
        m_pending.erase(it);
      }
    }
    if(pending && pending->timer)
      pending->timer->cancel();
  }

  if(!ev->error && ev->header.buffers)
    ev->decode_buffers();

  if(pending) {
    pending->hdlr->handle(ptr(), ev);
    delete pending;
  } else {
    run(ev);
  }
}





ConnHandlerPlain::ConnHandlerPlain(AppContext::Ptr& app_ctx, 
                                   SocketPlain& socket)
                                  : ConnHandler(app_ctx), 
                                    m_sock(std::move(socket)) {
  m_sock.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
}

ConnHandlerPlain::~ConnHandlerPlain() {
  if(is_open()) { 
    asio::error_code ec;
    m_sock.close(ec);
  }
}

void ConnHandlerPlain::do_close() {
  if(connected)
    close();
}

void ConnHandlerPlain::close() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(!connected)
      return;
    connected = false;
  }
  if(m_sock.is_open()) {
    asio::error_code ec;
    m_sock.cancel(ec);
    m_sock.close(ec);
  }
  disconnected();
  ConnHandler::do_close();
}

bool ConnHandlerPlain::is_open() const noexcept {
  return connected && m_sock.is_open();
}

SocketLayer* ConnHandlerPlain::socket_layer() noexcept {
  return &m_sock.lowest_layer();
}

void ConnHandlerPlain::do_async_write(
        const std::vector<asio::const_buffer>& buffers,
        const std::function<void(const asio::error_code&, uint32_t)>& hdlr)
        noexcept {
  asio::async_write(m_sock, buffers, hdlr);
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        const std::function<void(const asio::error_code&, size_t)>& hdlr)
        noexcept {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), hdlr);
}

void ConnHandlerPlain::read(uint8_t** bufp, size_t* remainp, 
                            asio::error_code& ec) { // unused
  size_t read = 0;
  do {
    read = m_sock.read_some(
      asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}




ConnHandlerSSL::ConnHandlerSSL(AppContext::Ptr& app_ctx, 
                               asio::ssl::context& ssl_ctx,
                               SocketPlain& socket)
                              : ConnHandler(app_ctx), 
                                m_sock(std::move(socket), ssl_ctx),
                                m_strand(m_sock.get_executor()) {
  m_sock.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
}

ConnHandlerSSL::~ConnHandlerSSL() { 
  if(is_open()) {
    asio::error_code ec;
    m_sock.lowest_layer().close(ec);
  }
}

void ConnHandlerSSL::do_close() {
  if(connected)
    close();
}

void ConnHandlerSSL::close() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(!connected)
      return;
    connected = false;
  }
  if(m_sock.lowest_layer().is_open()) {
    asio::error_code ec;
    m_sock.lowest_layer().cancel(ec);
    //m_sock.shutdown(ec);
    m_sock.lowest_layer().close(ec);
  }
  disconnected();
  ConnHandler::do_close();

  /* ssl-segment (if protocol is shutdown)
  if(m_sock.lowest_layer().is_open()) {
    m_sock.async_shutdown([this, ptr=ptr()](asio::error_code ec) {
      if(m_sock.lowest_layer().is_open()) {
        m_sock.lowest_layer().cancel(ec);
        m_sock.lowest_layer().close(ec);
      }
      disconnected();
      ConnHandler::do_close();
    });
  } else {
    disconnected();
    ConnHandler::do_close();
  } */
}

bool ConnHandlerSSL::is_open() const noexcept {
  return connected && m_sock.lowest_layer().is_open();
}

void ConnHandlerSSL::handshake(
                  SocketSSL::handshake_type typ,
                  const std::function<void(const asio::error_code&)>& cb) 
                  noexcept {
  /* any options ?
  asio::error_code ec;
  m_sock.lowest_layer().non_blocking(true, ec);
  if(ec)
    cb(ec);
  else  
  */
  m_sock.async_handshake(typ, cb);
}

void ConnHandlerSSL::handshake(
                  SocketSSL::handshake_type typ,
                  asio::error_code& ec) 
                  noexcept {
  /*
  m_sock.lowest_layer().non_blocking(true, ec);
  if(!ec)
  */
  m_sock.handshake(typ, ec);
}

void ConnHandlerSSL::set_verify(
        const std::function<bool(bool, asio::ssl::verify_context&)>& cb) 
        noexcept {
  m_sock.set_verify_callback(cb);
}


SocketLayer* ConnHandlerSSL::socket_layer() noexcept {
  return &m_sock.lowest_layer();
}
  
void ConnHandlerSSL::do_async_write(
        const std::vector<asio::const_buffer>& buffers,
        const std::function<void(const asio::error_code&, uint32_t)>& hdlr)
        noexcept {
  asio::async_write(
    m_sock, buffers, 
    asio::bind_executor(m_strand, hdlr)
  );
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        const std::function<void(const asio::error_code&, size_t)>& hdlr)
        noexcept {
  asio::async_read(
    m_sock, asio::mutable_buffer(data, sz), 
    asio::bind_executor(m_strand, hdlr)
  );
}

void ConnHandlerSSL::read(uint8_t** bufp, size_t* remainp, 
                            asio::error_code& ec) { // unused
  size_t read = 0;
  do {
    read = m_sock.read_some(
      asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}
 


}}
