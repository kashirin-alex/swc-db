/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/Checksum.h"



namespace SWC { namespace Comm {



void ConnHandler::new_connection() {
  auto sock = socket_layer();
  endpoint_remote = sock->remote_endpoint();
  endpoint_local = sock->local_endpoint();
  SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "New-"); );

  app_ctx->handle_established(ptr());

  read();
}

size_t ConnHandler::pending_read() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_pending.size();
}

void ConnHandler::do_close_run() noexcept {
  app_ctx->handle_disconnect(ptr());
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

bool ConnHandler::send_response(Buffers::Ptr cbuf,
                                DispatchHandler::Ptr hdlr) noexcept {
  if(!connected || cbuf->expired())
    return false;
  cbuf->header.flags &= Header::FLAG_REQUEST_MASK;
  write_or_queue(Outgoing(std::move(cbuf), std::move(hdlr)));
  return true;
}

bool ConnHandler::send_request(Buffers::Ptr cbuf,
                               DispatchHandler::Ptr hdlr) {
  if(!connected)
    return false;
  cbuf->header.flags |= Header::FLAG_REQUEST_BIT;
  write_or_queue(Outgoing(std::move(cbuf), std::move(hdlr)));
  return true;
}

void ConnHandler::print(std::ostream& out) const {
  out << "Connection(encoder=" << Core::Encoder::to_string(get_encoder())
      << " remote=" << endpoint_remote
      << " local=" << endpoint_local
      << ' ' << (is_secure() ? "SECURE" : "PLAIN");
  if(!is_open())
    out << " CLOSED";
  out << ')';
}

void ConnHandler::write_or_queue(ConnHandler::Outgoing&& outgoing) {
  outgoing.cbuf->prepare(get_encoder());

  if(m_outgoing.activating(outgoing))
    write(outgoing);
}

void ConnHandler::write_next() {
  Outgoing outgoing;
  if(!m_outgoing.deactivating(outgoing))
    write(outgoing);
}

void ConnHandler::write(ConnHandler::Outgoing& outgoing) {
  auto cbuf = std::move(outgoing.cbuf);
  auto& header = cbuf->header;

  if(!outgoing.hdlr || header.flags & Header::FLAG_RESPONSE_IGNORE_BIT) {
    // send request/response without sent/rsp-ack
    if(cbuf->expired())
      return write_next();
    if(connected) do_async_write(
      cbuf->get_buffers(),
      [cbuf, conn=ptr()] (const asio::error_code& ec, uint32_t bytes) {
        if(ec) {
          conn->do_close();
        } else {
          conn->write_next();
        }
        if(bytes)
          conn->app_ctx->net_bytes_sent(conn, bytes);
      }
    );
    return;
  }

  if(!(header.flags & Header::FLAG_REQUEST_BIT)) {
    //if(!header.timeout_ms) {
      // send_response with sent-ack
      if(cbuf->expired())
        return write_next();

      if(connected) do_async_write(
        cbuf->get_buffers(),
        [cbuf, hdlr=std::move(outgoing.hdlr), conn=ptr()]
        (const asio::error_code& ec, uint32_t bytes) {
          if(!ec)
            conn->write_next();

          auto ev = Event::make(ec ? Error::COMM_SEND_ERROR : Error::OK);
          ev->header.initialize_from_response(cbuf->header);
          hdlr->handle(conn, ev);
          if(ec)
            conn->do_close();
          if(bytes)
            conn->app_ctx->net_bytes_sent(conn, bytes);
        }
      );
    //} else {
      // send_response expect sent/timeout ack with m_sends
    return;
  }

  // send_request expect rsp/timeout with m_pending
  auto timer = header.timeout_ms
    ? new asio::high_resolution_timer(socket_layer()->get_executor())
    : nullptr;

  assign_id: {
    Core::MutexSptd::scope lock(m_mutex);
    if(header.id) {
      m_next_req_id = header.id;
      header.id = 0;
    } else if(!++m_next_req_id) {
      ++m_next_req_id;
    }
    auto r = m_pending.emplace(m_next_req_id, Pending());
    if(!r.second)
      goto assign_id;
    header.id = m_next_req_id;
    r.first->second.hdlr = std::move(outgoing.hdlr);

    if(timer) {
      r.first->second.timer = timer;
      timer->expires_after(
        std::chrono::milliseconds(header.timeout_ms));
      timer->async_wait(
        [cbuf, conn=ptr()] (const asio::error_code& ec) {
          if(ec == asio::error::operation_aborted)
            return;
          auto ev = Event::make(Error::REQUEST_TIMEOUT);
          ev->header.initialize_from_request(cbuf->header);
          ev->header.flags &= Header::FLAG_REQUEST_MASK;
          conn->run_pending(ev);
        }
      );
    }
  }

  if(!connected)
    return do_close();

  do_async_write(
    cbuf->get_buffers(),
    [cbuf, conn=ptr()] (const asio::error_code& ec, uint32_t bytes) {
      if(ec) {
        conn->do_close();
      } else {
        conn->write_next();
      }
      if(bytes)
        conn->app_ctx->net_bytes_sent(conn, bytes);
    }
  );
}

void ConnHandler::read() noexcept {
  if(!connected)
    return;

  m_recv_bytes = 0;
  do_async_read(
    _buf_header,
    Header::PREFIX_LENGTH,
    [ptr=ptr()] (const asio::error_code& ec, size_t filled) {
      ptr->recved_header_pre(ec, filled);
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_header_pre(const asio::error_code& ec, size_t filled) {
  m_recv_bytes += filled;
  if(ec || filled != Header::PREFIX_LENGTH)
    return do_close_recv();

  auto ev = Event::make(Error::OK);
  try {
    const uint8_t* buf = _buf_header;
    ev->header.decode_prefix(&buf, &filled);
  } catch(...) {
    ev->header.header_len = 0;
  }

  if(!ev->header.header_len) {
    SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_PREFIX_TRUNCATED: remain=%lu",
             filled);
    return do_close_recv();
  }

  do_async_read(
    _buf_header + Header::PREFIX_LENGTH,
    ev->header.header_len - Header::PREFIX_LENGTH,
    [ev, ptr=ptr()](const asio::error_code& ec, size_t filled) {
      ptr->recved_header(ev, ec, filled);
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_header(const Event::Ptr& ev, asio::error_code ec,
                                size_t filled) {
  m_recv_bytes += filled;
  if(!ec) {
    if(filled + Header::PREFIX_LENGTH != ev->header.header_len) {
      ec = asio::error::eof;
    } else {
      filled = ev->header.header_len;
      try {
        const uint8_t* buf = _buf_header;
        ev->header.decode(&buf, &filled);
      } catch(...) {
        ec = asio::error::eof;
      }
    }
  }
  if(ec) {
    SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_TRUNCATED: len=%d",
             ev->header.header_len);
    do_close_recv();
  } else if(ev->header.buffers) {
    recv_buffers(ev);
  } else {
    received(ev);
  }
}

void ConnHandler::recv_buffers(const Event::Ptr& ev) {

  StaticBuffer* buffer;
  size_t remain;
  if(!ev->data.size) {
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
    [ev, ptr=ptr()](const asio::error_code& ec, size_t filled) {
      ptr->recved_buffer(ev, ec, filled);
    }
  );
}

SWC_SHOULD_INLINE
void ConnHandler::recved_buffer(const Event::Ptr& ev,
                                asio::error_code ec, size_t filled) {
  m_recv_bytes += filled;
  if(!ec) {
    StaticBuffer* buffer;
    uint32_t checksum;
    if(!ev->data_ext.size) {
      buffer = &ev->data;
      checksum = ev->header.data.chksum;
    } else {
      buffer = &ev->data_ext;
      checksum = ev->header.data_ext.chksum;
    }

    if(filled != buffer->size ||
       !Core::checksum_i32_chk(checksum, buffer->base, buffer->size)) {
      ec = asio::error::eof;
    }
  }
  if(ec) {
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM
        << "read, REQUEST PAYLOAD_TRUNCATED: nbuff("
        << (bool(ev->data.size) + bool(ev->data_ext.size))
        << ") ";
      ev->print(SWC_LOG_OSTREAM);
    );
    do_close_recv();
  } else if(ev->header.buffers == bool(ev->data.size) +
                                  bool(ev->data_ext.size)) {
    received(ev);
  } else {
    recv_buffers(ev);
  }
}

void ConnHandler::received(const Event::Ptr& ev) noexcept {
  if(ev->header.flags & Header::FLAG_REQUEST_BIT)
    ev->received();

  app_ctx->net_bytes_received(ptr(), m_recv_bytes);

  read();

  try {
    run_pending(ev);
  } catch(...) {
    try {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR,
        print(SWC_LOG_OSTREAM << ' ');
        ev->print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << ' ' << e;
      );
    } catch(...) { }
  }
}

void ConnHandler::disconnected() noexcept {
  try {
    for(Outgoing outgoing; !m_outgoing.deactivating(outgoing); ) {
      if(outgoing.hdlr)
        outgoing.hdlr->handle_no_conn();
    }
    for(Pending pending; ;) {
      {
        Core::MutexSptd::scope lock(m_mutex);
        if(m_pending.empty())
          return;
        pending = std::move(m_pending.begin()->second);
        m_pending.erase(m_pending.begin());
      }
      if(pending.timer)
        pending.timer->cancel();
      pending.hdlr->handle_no_conn();
    }

  } catch(...) {
    try {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR,
        print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << ' ' << e;
      );
    } catch(...) { }
  }
}

void ConnHandler::run_pending(const Event::Ptr& ev) {
  Pending pending;
  if(ev->header.id && !(ev->header.flags & Header::FLAG_REQUEST_BIT)) {
    bool partial = ev->header.flags & Header::FLAG_RESPONSE_PARTIAL_BIT;
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = m_pending.find(ev->header.id);
      if(it != m_pending.end()) {
        if(partial) {
          pending.hdlr = it->second.hdlr;
        } else {
          pending = std::move(it->second);
          m_pending.erase(it);
        }
      }
    }
    if(pending.timer)
      pending.timer->cancel();
  }

  if(!ev->error && ev->header.buffers)
    ev->decode_buffers();

  if(pending.hdlr) {
    pending.hdlr->handle(ptr(), ev);
  } else {
    app_ctx->handle(ptr(), ev);
  }
}





ConnHandlerPlain::Ptr
ConnHandlerPlain::make(AppContext::Ptr& app_ctx, SocketPlain& socket) {
  return ConnHandlerPlain::Ptr(new ConnHandlerPlain(app_ctx, socket));
}

ConnHandlerPlain::ConnHandlerPlain(AppContext::Ptr& app_ctx,
                                   SocketPlain& socket) noexcept
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

void ConnHandlerPlain::do_close() noexcept {
  bool at = true;
  if(connected.compare_exchange_weak(at, false)) {
    if(m_sock.is_open()) {
      asio::error_code ec;
      m_sock.cancel(ec);
      m_sock.close(ec);
    }
    do_close_run();
  }
  disconnected();
}

bool ConnHandlerPlain::is_open() const noexcept {
  return connected && m_sock.is_open();
}

SocketLayer* ConnHandlerPlain::socket_layer() noexcept {
  return &m_sock.lowest_layer();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
void ConnHandlerPlain::do_async_write(
        std::vector<asio::const_buffer>&& buffers,
        std::function<void(const asio::error_code&, uint32_t)>&& hdlr)
        noexcept {
  asio::async_write(m_sock, std::move(buffers), std::move(hdlr));
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        std::function<void(const asio::error_code&, size_t)>&& hdlr)
        noexcept {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), std::move(hdlr));
}
#pragma GCC diagnostic pop

void ConnHandlerPlain::read(uint8_t** bufp, size_t* remainp,
                            asio::error_code& ec) { // unused
  size_t read = 0;
  do {
    read = m_sock.read_some(
      asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}




ConnHandlerSSL::Ptr
ConnHandlerSSL::make(AppContext::Ptr& app_ctx, asio::ssl::context& ssl_ctx,
                     SocketPlain& socket) {
  return ConnHandlerSSL::Ptr(new ConnHandlerSSL(app_ctx, ssl_ctx, socket));
}

ConnHandlerSSL::ConnHandlerSSL(AppContext::Ptr& app_ctx,
                               asio::ssl::context& ssl_ctx,
                               SocketPlain& socket) noexcept
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

void ConnHandlerSSL::do_close() noexcept {
  bool at = true;
  if(connected.compare_exchange_weak(at, false)) {
    if(m_sock.lowest_layer().is_open()) {
      asio::error_code ec;
      m_sock.lowest_layer().cancel(ec);
      //m_sock.shutdown(ec);
      m_sock.lowest_layer().close(ec);
    }
    do_close_run();
  }
  disconnected();

  /* ssl-segment (if protocol is shutdown)
  if(m_sock.lowest_layer().is_open()) {
    m_sock.async_shutdown([this, ptr=ptr()](asio::error_code ec) {
      if(m_sock.lowest_layer().is_open()) {
        m_sock.lowest_layer().cancel(ec);
        m_sock.lowest_layer().close(ec);
      }
      do_close_run();
    });
  } else {
    do_close_run();
  } */
}

bool ConnHandlerSSL::is_open() const noexcept {
  return connected && m_sock.lowest_layer().is_open();
}

void ConnHandlerSSL::handshake(
                  SocketSSL::handshake_type typ,
                  std::function<void(const asio::error_code&)>&& cb)
                  noexcept {
  /* any options ?
  asio::error_code ec;
  m_sock.lowest_layer().non_blocking(true, ec);
  if(ec)
    cb(ec);
  else
  */
  m_sock.async_handshake(typ, std::move(cb));
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
        std::function<bool(bool, asio::ssl::verify_context&)>&& cb)
        noexcept {
  m_sock.set_verify_callback(std::move(cb));
}


SocketLayer* ConnHandlerSSL::socket_layer() noexcept {
  return &m_sock.lowest_layer();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
void ConnHandlerSSL::do_async_write(
        std::vector<asio::const_buffer>&& buffers,
        std::function<void(const asio::error_code&, uint32_t)>&& hdlr)
        noexcept {
  asio::async_write(
    m_sock, std::move(buffers),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        std::function<void(const asio::error_code&, size_t)>&& hdlr)
        noexcept {
  asio::async_read(
    m_sock, asio::mutable_buffer(data, sz),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}
#pragma GCC diagnostic pop

void ConnHandlerSSL::read(uint8_t** bufp, size_t* remainp,
                            asio::error_code& ec) { // unused
  size_t read = 0;
  do {
    read = m_sock.read_some(
      asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}



}}
