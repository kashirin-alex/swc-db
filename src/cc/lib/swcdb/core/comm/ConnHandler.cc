/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/Checksum.h"

#include "swcdb/core/comm/ConnHandler.h"


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
                                : cbuf(cbuf), hdlr(hdlr) { }
ConnHandler::Outgoing::~Outgoing() { }

ConnHandler::ConnHandler(AppContext::Ptr app_ctx, Socket& socket) 
                        : app_ctx(app_ctx), m_sock(std::move(socket)), 
                          m_next_req_id(0) {
}

ConnHandlerPtr ConnHandler::ptr() {
  return shared_from_this();
}

ConnHandler::~ConnHandler() { 
  do_close();
}


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
  {
    LockAtomic::Unique::Scope lock(m_mutex);

    endpoint_remote = m_sock.remote_endpoint();
    endpoint_local = m_sock.local_endpoint();
    SWC_LOGF(LOG_DEBUG, "new_connection local=%s, remote=%s, executor=%d",
            endpoint_local_str().c_str(), endpoint_remote_str().c_str(),
            (size_t)&m_sock.get_executor().context());
  }         
  auto ev = Event::make(Event::Type::ESTABLISHED, Error::OK);
  run(ev); 
}

const bool ConnHandler::is_open() {
  return m_err == Error::OK && m_sock.is_open();
}

void ConnHandler::close() {
  if(is_open()) {
    LockAtomic::Unique::Scope lock(m_mutex);
    try{m_sock.close();}catch(...){}
  }
  m_err = Error::COMM_NOT_CONNECTED;
  disconnected();
}

const size_t ConnHandler::pending_read() {
  LockAtomic::Unique::Scope lock(m_mutex_reading);
  return m_pending.size();
}

const size_t ConnHandler::pending_write() {
  LockAtomic::Unique::Scope lock(m_mutex);
  return m_outgoing.size();
}

const bool ConnHandler::due() {
  return pending_read() > 0 || pending_write() > 0;
}

void ConnHandler::run(Event::Ptr& ev) {
  if(app_ctx != nullptr) 
    // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
    app_ctx->handle(ptr(), ev); 
}

void ConnHandler::do_close() {
  if(m_err == Error::OK) {
    close();
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

/*
int ConnHandler::send_response(CommBuf::Ptr &cbuf, uint32_t timeout_ms){
    if(m_err != Error::OK)
      return m_err;

    int e = Error::OK;
    cbuf->header.flags &= CommHeader::FLAGS_MASK_REQUEST;

    std::vector<asio::const_buffer> buffers;
    cbuf->get(buffers);

    std::future<size_t> f = asio::async_write(
      m_sock, buffers, asio::use_future);

    std::future_status status = f.wait_for(
      std::chrono::milliseconds(timeout_ms));

    if (status == std::future_status::ready){
      try {
        f.get();
      } catch (const std::exception& ex) {
        e = (m_err != Error::OK? m_err.load() : Error::COMM_BROKEN_CONNECTION);
      }
    } else {
      e = Error::REQUEST_TIMEOUT;
    }
    read_pending();
    return e;
}

int send_request(uint32_t timeout_ms, CommBuf::Ptr &cbuf, 
                          DispatchHandler::Ptr hdlr) {
    cbuf->header.timeout_ms = timeout_ms;
    return send_request(cbuf, hdlr);  
}
*/

const int ConnHandler::send_request(CommBuf::Ptr &cbuf, 
                                    DispatchHandler::Ptr hdlr) {
  if(m_err != Error::OK)
    return m_err;

  cbuf->header.flags |= CommHeader::FLAGS_BIT_REQUEST;
  if(cbuf->header.id == 0)
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
  LockAtomic::Unique::Scope lock(m_mutex_reading);  
  while(m_pending.find(
    ++m_next_req_id == 0 ? ++m_next_req_id : m_next_req_id
    ) != m_pending.end()
  );
  return m_next_req_id;
}

asio::high_resolution_timer* ConnHandler::get_timer(const CommHeader& header) {
  if(!header.timeout_ms)
    return nullptr;

  auto tm = new asio::high_resolution_timer(
    m_sock.get_executor(), std::chrono::milliseconds(header.timeout_ms));

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
    LockAtomic::Unique::Scope lock(m_mutex);  
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
    LockAtomic::Unique::Scope lock(m_mutex);
    m_writing = !m_outgoing.empty();
    if(!m_writing) 
      return;
    data = m_outgoing.front();
    m_outgoing.pop();
  }
  write(data); 
}
  
void ConnHandler::write(ConnHandler::Outgoing* data) {

  if(data->cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST) {
    LockAtomic::Unique::Scope lock(m_mutex_reading);
    m_pending.insert(std::make_pair(
      data->cbuf->header.id, 
      new ConnHandler::PendingRsp(data->hdlr, get_timer(data->cbuf->header))
    ));
  }
    
  std::vector<asio::const_buffer> buffers;
  data->cbuf->get(buffers);
    
  asio::async_write(
    m_sock, 
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
    LockAtomic::Unique::Scope lock(m_mutex_reading);
    if(m_err || m_reading)
      return;
    m_reading = true;
  }

  Event::Ptr ev = Event::make(Event::Type::MESSAGE, Error::OK);
  uint8_t* data = new uint8_t[CommHeader::PREFIX_LENGTH+1];

  asio::async_read(
    m_sock, 
    asio::mutable_buffer(data, CommHeader::PREFIX_LENGTH+1),
    [ev, data, ptr=ptr()](const asio::error_code e, size_t filled) {
      return ptr->read_condition_hdlr(ev, data, e, filled);
    },
    [ev, ptr=ptr()](const asio::error_code e, size_t filled) {
      ptr->received(ev, e);
    }
  );
}
  
size_t ConnHandler::read_condition_hdlr(const Event::Ptr& ev, uint8_t* data,
                                        const asio::error_code e, 
                                        size_t filled) { 
  size_t remain;
  asio::error_code ec = e;                      
  if(!ec) 
    remain = filled < CommHeader::PREFIX_LENGTH ? 
              CommHeader::PREFIX_LENGTH-filled
              : read_condition(ev, data, ec);
  if(ec) {
    remain = 0;
    do_close();
  }
  if(remain == 0) 
    delete [] data;
  return remain;
}
  
size_t ConnHandler::read_condition(const Event::Ptr& ev, uint8_t* data, 
                                   asio::error_code &ec) {
  size_t remain = CommHeader::PREFIX_LENGTH;
  const uint8_t* ptr = data;
  size_t  read = 0;
  uint8_t* bufp;
    
  try {
    ev->header.decode_prefix(&ptr, &remain);

    uint8_t buf_header[ev->header.header_len];
    bufp = buf_header;
    ptr = data;
    for(uint8_t n=0; n<CommHeader::PREFIX_LENGTH; n++)
      *bufp++ = *ptr++;

    remain = ev->header.header_len - CommHeader::PREFIX_LENGTH;
    do {
      read = m_sock.read_some(asio::mutable_buffer(bufp+=read, remain), ec);
    } while(!ec && (remain -= read));  
      
    ptr = buf_header;
    remain = ev->header.header_len;
    ev->header.decode(&ptr, &remain);
  } catch(...){
    ec = asio::error::eof;
  }
  if(ec) {
    ec = asio::error::eof;
    ev->type = Event::Type::ERROR;
    ev->error = Error::REQUEST_TRUNCATED_HEADER;
    ev->header = CommHeader();
    SWC_LOGF(LOG_WARN, "read, REQUEST HEADER_TRUNCATED: remain=%d", remain);
    return (size_t)0;
  }
  if(!ev->header.buffers)
    return (size_t)0;

  uint32_t checksum;
  for(uint8_t n=0; n < ev->header.buffers; n++) {
    StaticBuffer& buffer = n == 0 ? ev->data :  ev->data_ext;
    if(n == 0) {
      remain = ev->header.data_size;
      checksum = ev->header.data_chksum;
    } else {
      remain = ev->header.data_ext_size;
      checksum = ev->header.data_ext_chksum;
    }
    buffer.reallocate(remain);

    bufp = buffer.base;
    read = 0;
    do {
      read = m_sock.read_some(asio::mutable_buffer(bufp+=read, remain), ec);
    } while(!ec && (remain -= read));
    if(ec) 
      break;
    if(!checksum_i32_chk(checksum, buffer.base, buffer.size)) {
      ec = asio::error::eof;
      break;
    }
  }

  if(ec) {
    ec = asio::error::eof;
    ev->error = Error::REQUEST_TRUNCATED_PAYLOAD;
    ev->data.free();
    ev->data_ext.free();
    SWC_LOGF(LOG_WARN, "read, REQUEST PAYLOAD_TRUNCATED: error=(%s) %s", 
             ec.message().c_str(), ev->to_str().c_str());
  }
  return (size_t)0;
}

void ConnHandler::received(const Event::Ptr& ev, const asio::error_code ec) {
  if(ec) {
    do_close();
    return;
  }
    
  ev->arrival_time = ClockT::now();
  bool more;
  {
    LockAtomic::Unique::Scope lock(m_mutex_reading);
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
      LockAtomic::Unique::Scope lock(m_mutex_reading);
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
  if(ev->header.id == 0) {
    run(ev);
    return;
  }

  ConnHandler::PendingRsp* pending = nullptr;
  {
    LockAtomic::Unique::Scope lock(m_mutex_reading);
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


}