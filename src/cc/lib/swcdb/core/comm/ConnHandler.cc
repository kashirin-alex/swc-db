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

  if(m_outgoing.activating(std::move(outgoing)))
    write(outgoing);
}

void ConnHandler::write_next() {
  Outgoing outgoing;
  if(!m_outgoing.deactivating(outgoing))
    write(outgoing);
}



struct ConnHandler::Sender_noAck final {
  ConnHandlerPtr        conn;
  Buffers::Ptr          cbuf;
  SWC_CAN_INLINE
  Sender_noAck(ConnHandlerPtr&& a_conn, const Buffers::Ptr& a_cbuf) noexcept
              : conn(std::move(a_conn)), cbuf(a_cbuf) {
  }
  SWC_CAN_INLINE
  Sender_noAck(Sender_noAck&& other) noexcept
            : conn(std::move(other.conn)), cbuf(std::move(other.cbuf)) {
  }
  Sender_noAck(const Sender_noAck&) = delete;
  Sender_noAck& operator=(Sender_noAck&&) = delete;
  Sender_noAck& operator=(const Sender_noAck&) = delete;
  ~Sender_noAck() noexcept { }
  void operator()(const asio::error_code& ec, uint32_t bytes) noexcept {
    try {
      if(!ec)
        conn->write_next();
      if(bytes)
        conn->app_ctx->net_bytes_sent(conn, bytes);
      if(!ec)
        return;
    } catch(...) { }
    conn->do_close();
  }
};


struct ConnHandler::Sender_Ack final {
  ConnHandlerPtr        conn;
  Buffers::Ptr          cbuf;
  DispatchHandler::Ptr  hdlr;
  SWC_CAN_INLINE
  Sender_Ack(ConnHandlerPtr&& a_conn, const Buffers::Ptr& a_cbuf,
             DispatchHandler::Ptr&& a_hdlr) noexcept
            : conn(std::move(a_conn)),
              cbuf(a_cbuf),
              hdlr(std::move(a_hdlr)) {
  }
  SWC_CAN_INLINE
  Sender_Ack(Sender_Ack&& other) noexcept
            : conn(std::move(other.conn)),
              cbuf(std::move(other.cbuf)),
              hdlr(std::move(other.hdlr)) {
  }
  Sender_Ack(const Sender_Ack&) = delete;
  Sender_Ack& operator=(Sender_Ack&&) = delete;
  Sender_Ack& operator=(const Sender_Ack&) = delete;
  ~Sender_Ack() noexcept { }
  void operator()(const asio::error_code& ec, uint32_t bytes) noexcept {
    try {
      if(!ec)
        conn->write_next();
      auto ev = Event::make(ec ? Error::COMM_SEND_ERROR : Error::OK);
      ev->header.initialize_from_response(cbuf->header);
      hdlr->handle(conn, ev);
      if(bytes)
        conn->app_ctx->net_bytes_sent(conn, bytes);
      if(!ec)
        return;
    } catch(...) { }
    conn->do_close();
  }
};



void ConnHandler::write(ConnHandler::Outgoing& outgoing) {
  auto cbuf = std::move(outgoing.cbuf);
  auto& header = cbuf->header;

  if(!outgoing.hdlr || header.flags & Header::FLAG_RESPONSE_IGNORE_BIT) {
    // send request/response without sent/rsp-ack
    if(cbuf->expired())
      return write_next();
    if(connected)
      do_async_write(
        cbuf->get_buffers(),
        Sender_noAck(ptr(), cbuf)
      );
    return;
  }

  if(!(header.flags & Header::FLAG_REQUEST_BIT)) {
    //if(!header.timeout_ms) {
      // send_response with sent-ack
      if(cbuf->expired())
        return write_next();

      if(connected) {
        do_async_write(
          cbuf->get_buffers(),
          Sender_Ack(ptr(), cbuf, std::move(outgoing.hdlr))
        );
    } //else {
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
      struct TimerTask {
        ConnHandlerPtr        conn;
        Buffers::Ptr          cbuf;
        TimerTask(ConnHandlerPtr&& a_conn, const Buffers::Ptr& a_cbuf)
                  noexcept : conn(std::move(a_conn)), cbuf(a_cbuf) { }
        SWC_CAN_INLINE
        TimerTask(TimerTask&& other) noexcept
                  : conn(std::move(other.conn)),
                    cbuf(std::move(other.cbuf)) {
        }
        TimerTask(const TimerTask&) = delete;
        TimerTask& operator=(TimerTask&&) = delete;
        TimerTask& operator=(const TimerTask&) = delete;
        ~TimerTask() noexcept { }
        void operator()(const asio::error_code& ec) {
          if(ec == asio::error::operation_aborted)
            return;
          auto ev = Event::make(Error::REQUEST_TIMEOUT);
          ev->header.initialize_from_request(cbuf->header);
          ev->header.flags &= Header::FLAG_REQUEST_MASK;
          conn->run_pending(std::move(ev));
        }
      };
      timer->async_wait(TimerTask(ptr(), cbuf));
    }
  }

  if(!connected)
    return do_close();

  do_async_write(
    cbuf->get_buffers(),
    Sender_noAck(ptr(), cbuf)
  );
}



struct ConnHandler::Receiver_HeaderPrefix final {
  ConnHandlerPtr  conn;
  SWC_CAN_INLINE
  Receiver_HeaderPrefix(ConnHandlerPtr&& a_conn) noexcept
                        : conn(std::move(a_conn)) { }
  SWC_CAN_INLINE
  Receiver_HeaderPrefix(Receiver_HeaderPrefix&& other) noexcept
                        : conn(std::move(other.conn)) { }
  Receiver_HeaderPrefix(const Receiver_HeaderPrefix&) = delete;
  Receiver_HeaderPrefix& operator=(Receiver_HeaderPrefix&&) = delete;
  Receiver_HeaderPrefix& operator=(const Receiver_HeaderPrefix&) = delete;
  ~Receiver_HeaderPrefix() noexcept { }
  void operator()(const asio::error_code& ec, size_t filled) noexcept;
};

struct ConnHandler::Receiver_Header final {
  ConnHandlerPtr  conn;
  Event::Ptr      ev;
  SWC_CAN_INLINE
  Receiver_Header(const ConnHandlerPtr& a_conn, Event::Ptr&& a_ev) noexcept
                 : conn(a_conn), ev(std::move(a_ev)) { }
  SWC_CAN_INLINE
  Receiver_Header(Receiver_Header&& other) noexcept
                  : conn(std::move(other.conn)), ev(std::move(other.ev)) { }
  Receiver_Header(const Receiver_Header&) = delete;
  Receiver_Header& operator=(Receiver_Header&&) = delete;
  Receiver_Header& operator=(const Receiver_Header&) = delete;
  ~Receiver_Header() noexcept { }
  void operator()(asio::error_code ec, size_t filled) noexcept;
};

struct ConnHandler::Receiver_Buffer final {
  ConnHandlerPtr  conn;
  Event::Ptr      ev;
  SWC_CAN_INLINE
  Receiver_Buffer(ConnHandlerPtr&& a_conn, Event::Ptr&& a_ev) noexcept
                  : conn(std::move(a_conn)), ev(std::move(a_ev)) { }
  SWC_CAN_INLINE
  Receiver_Buffer(Receiver_Buffer&& other) noexcept
                  : conn(std::move(other.conn)), ev(std::move(other.ev)) { }
  Receiver_Buffer(const Receiver_Buffer&) = delete;
  Receiver_Buffer& operator=(Receiver_Buffer&&) = delete;
  Receiver_Buffer& operator=(const Receiver_Buffer&) = delete;
  ~Receiver_Buffer() noexcept { }
  void operator()(asio::error_code ec, size_t filled) noexcept;
};


void ConnHandler::Receiver_HeaderPrefix::operator()(
                        const asio::error_code& ec, size_t filled) noexcept {
  try {
    conn->m_recv_bytes += filled;
    if(ec || filled != Header::PREFIX_LENGTH)
      goto _quit;

    auto ev = Event::make(Error::OK);
    try {
      const uint8_t* buf = conn->_buf_header;
      ev->header.decode_prefix(&buf, &filled);
    } catch(...) {
      ev->header.header_len = 0;
    }

    if(!ev->header.header_len) {
      SWC_LOGF(LOG_WARN,
        "read, REQUEST HEADER_PREFIX_TRUNCATED: remain=" SWC_FMT_LU,
        filled);
      goto _quit;
    }
    filled = ev->header.header_len - Header::PREFIX_LENGTH;
    conn->do_async_read(
      conn->_buf_header + Header::PREFIX_LENGTH,
      filled,
      Receiver_Header(conn, std::move(ev))
    );
    return;
  } catch(...) {
  }
  _quit:
    conn->do_close_recv();
}

void ConnHandler::Receiver_Header::operator()(
                        asio::error_code ec, size_t filled) noexcept {
  try {
    conn->m_recv_bytes += filled;
    if(!ec) {
      if(filled + Header::PREFIX_LENGTH != ev->header.header_len) {
        ec = asio::error::eof;
      } else {
        filled = ev->header.header_len;
        try {
          const uint8_t* buf = conn->_buf_header;
          ev->header.decode(&buf, &filled);
        } catch(...) {
          ec = asio::error::eof;
        }
      }
    }
    if(ec) {
      SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_TRUNCATED: len=%d",
               ev->header.header_len);
      goto _quit;
    } else if(ev->header.buffers) {
      conn->recv_buffers(std::move(ev));
    } else {
      conn->received(std::move(ev));
    }
    return;
  } catch(...) {
  }
  _quit:
    conn->do_close_recv();
}

void ConnHandler::Receiver_Buffer::operator()(asio::error_code ec,
                                              size_t filled) noexcept {
  try {
    conn->m_recv_bytes += filled;
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
        SWC_LOG_OSTREAM << "read, REQUEST PAYLOAD_TRUNCATED: nbuff("
          << (bool(ev->data.size) + bool(ev->data_ext.size)) << ") ";
        ev->print(SWC_LOG_OSTREAM);
      );
      goto _quit;
    } else if(ev->header.buffers == bool(ev->data.size) +
                                    bool(ev->data_ext.size)) {
      conn->received(std::move(ev));
    } else {
      conn->recv_buffers(std::move(ev));
    }
    return;
  } catch(...) {
  }
  _quit:
    conn->do_close_recv();
}



void ConnHandler::read() noexcept {
  if(connected) {
    m_recv_bytes = 0;
    do_async_read(
      _buf_header, Header::PREFIX_LENGTH,
      Receiver_HeaderPrefix(ptr())
    );
  }
}

void ConnHandler::recv_buffers(Event::Ptr&& ev) {
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
    buffer->base, remain,
    Receiver_Buffer(ptr(), std::move(ev))
  );
}

void ConnHandler::received(Event::Ptr&& ev) noexcept {
  if(ev->header.flags & Header::FLAG_REQUEST_BIT)
    ev->received();

  try {
    run_pending(std::move(ev));
  } catch(...) {
    try {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR,
        print(SWC_LOG_OSTREAM << ' ');
        if(ev) ev->print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << ' ' << e;
      );
    } catch(...) { }
  }

  app_ctx->net_bytes_received(ptr(), m_recv_bytes);
  read();
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
        m_pending.erase(m_pending.cbegin());
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

void ConnHandler::run_pending(Event::Ptr&& ev) {
  Pending pending;
  if(ev->header.id && !(ev->header.flags & Header::FLAG_REQUEST_BIT)) {
    bool partial = ev->header.flags & Header::FLAG_RESPONSE_PARTIAL_BIT;
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = m_pending.find(ev->header.id);
      if(it != m_pending.cend()) {
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

  if(pending.hdlr) {
    struct Task {
      ConnHandlerPtr        conn;
      DispatchHandler::Ptr  hdlr;
      Event::Ptr            ev;
      Task(ConnHandlerPtr&& a_conn, DispatchHandler::Ptr&& a_hdlr,
           Event::Ptr&& a_ev) noexcept
          : conn(std::move(a_conn)),
            hdlr(std::move(a_hdlr)),
            ev(std::move(a_ev)) {
      }
      SWC_CAN_INLINE
      Task(Task&& other) noexcept
                : conn(std::move(other.conn)),
                  hdlr(std::move(other.hdlr)),
                  ev(std::move(other.ev)) {
      }
      Task(const Task&) = delete;
      Task& operator=(Task&&) = delete;
      Task& operator=(const Task&) = delete;
      ~Task() noexcept { }
      void operator()() {
        if(!ev->error && ev->header.buffers)
          ev->decode_buffers();
        hdlr->handle(conn, ev);
      }
    };
    asio::post(
      socket_layer()->get_executor(),
      Task(ptr(), std::move(pending.hdlr), std::move(ev))
    );

  } else {
    struct Task {
      ConnHandlerPtr        conn;
      Event::Ptr            ev;
      Task(ConnHandlerPtr&& a_conn, Event::Ptr&& a_ev) noexcept
          : conn(std::move(a_conn)), ev(std::move(a_ev)) { }
      SWC_CAN_INLINE
      Task(Task&& other) noexcept
          : conn(std::move(other.conn)), ev(std::move(other.ev)) { }
      Task(const Task&) = delete;
      Task& operator=(Task&&) = delete;
      Task& operator=(const Task&) = delete;
      ~Task() noexcept { }
      void operator()() {
        if(!ev->error && ev->header.buffers)
          ev->decode_buffers();
        conn->app_ctx->handle(conn, ev);
      }
    };
    asio::post(socket_layer()->get_executor(), Task(ptr(), std::move(ev)));
  }
}





ConnHandlerPlain::Ptr
ConnHandlerPlain::make(AppContext::Ptr& app_ctx, SocketPlain& socket) {
  return ConnHandlerPlain::Ptr(new ConnHandlerPlain(app_ctx, socket));
}

ConnHandlerPlain::ConnHandlerPlain(AppContext::Ptr& a_app_ctx,
                                   SocketPlain& socket) noexcept
                                  : ConnHandler(a_app_ctx),
                                    m_sock(std::move(socket)) {
  m_sock.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
}

ConnHandlerPlain::~ConnHandlerPlain() noexcept {
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
#pragma GCC diagnostic ignored "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

void ConnHandlerPlain::do_async_write(
        Core::Vector<asio::const_buffer>&& buffers,
        ConnHandler::Sender_noAck&& hdlr)
        noexcept {
  asio::async_write(m_sock, std::move(buffers), std::move(hdlr));
}

void ConnHandlerPlain::do_async_write(
        Core::Vector<asio::const_buffer>&& buffers,
        ConnHandler::Sender_Ack&& hdlr)
        noexcept {
  asio::async_write(m_sock, std::move(buffers), std::move(hdlr));
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_HeaderPrefix&& hdlr) noexcept {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), std::move(hdlr));
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_Header&& hdlr) noexcept {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), std::move(hdlr));
}

void ConnHandlerPlain::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_Buffer&& hdlr) noexcept {
  asio::async_read(m_sock, asio::mutable_buffer(data, sz), std::move(hdlr));
}


void ConnHandlerPlain::read(uint8_t** bufp, size_t* remainp,
                            asio::error_code& ec) { // unused
  size_t read = 0;
  do {
    read = m_sock.read_some(
      asio::mutable_buffer((*bufp)+=read, *remainp), ec);
  } while(!ec && (*remainp -= read));
}

#pragma GCC diagnostic pop




ConnHandlerSSL::Ptr
ConnHandlerSSL::make(AppContext::Ptr& app_ctx, asio::ssl::context& ssl_ctx,
                     SocketPlain& socket) {
  return ConnHandlerSSL::Ptr(new ConnHandlerSSL(app_ctx, ssl_ctx, socket));
}

ConnHandlerSSL::ConnHandlerSSL(AppContext::Ptr& a_app_ctx,
                               asio::ssl::context& ssl_ctx,
                               SocketPlain& socket) noexcept
                              : ConnHandler(a_app_ctx),
                                m_sock(std::move(socket), ssl_ctx),
                                m_strand(m_sock.get_executor()) {
  m_sock.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
}

ConnHandlerSSL::~ConnHandlerSSL() noexcept {
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
                  asio::error_code& ec)
                  noexcept {
  /*
  m_sock.lowest_layer().non_blocking(true, ec);
  if(!ec)
  */
  m_sock.handshake(typ, ec);
}

SocketLayer* ConnHandlerSSL::socket_layer() noexcept {
  return &m_sock.lowest_layer();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

void ConnHandlerSSL::do_async_write(
        Core::Vector<asio::const_buffer>&& buffers,
        ConnHandler::Sender_noAck&& hdlr)
        noexcept {
  asio::async_write(
    m_sock, std::move(buffers),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}

void ConnHandlerSSL::do_async_write(
        Core::Vector<asio::const_buffer>&& buffers,
        ConnHandler::Sender_Ack&& hdlr)
        noexcept {
  asio::async_write(
    m_sock, std::move(buffers),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_HeaderPrefix&& hdlr) noexcept {
  asio::async_read(
    m_sock, asio::mutable_buffer(data, sz),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_Header&& hdlr) noexcept {
  asio::async_read(
    m_sock, asio::mutable_buffer(data, sz),
    asio::bind_executor(m_strand, std::move(hdlr))
  );
}

void ConnHandlerSSL::do_async_read(
        uint8_t* data, uint32_t sz,
        ConnHandler::Receiver_Buffer&& hdlr) noexcept {
  asio::async_read(
    m_sock, asio::mutable_buffer(data, sz),
    asio::bind_executor(m_strand, std::move(hdlr))
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

#pragma GCC diagnostic pop


}}
