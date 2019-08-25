/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandler_h
#define swc_core_comm_ConnHandler_h

#include <asio.hpp>
#include "swcdb/lib/core/comm/Resolver.h"

namespace SWC { 

typedef std::shared_ptr<asio::io_context> IOCtxPtr;
typedef std::shared_ptr<asio::ip::tcp::socket> SocketPtr;
typedef std::shared_ptr<asio::high_resolution_timer> TimerPtr;

// forward declarations
class ConnHandler;
typedef std::shared_ptr<ConnHandler> ConnHandlerPtr;

}
 
#include <memory>
#include "DispatchHandler.h"

#include "Event.h"
#include "CommBuf.h"
#include "Protocol.h"
#include <queue>

namespace SWC { 


inline size_t endpoints_hash(EndPoints endpoints){
  std::string s;
  for(auto e : endpoints){
    s.append(e.address().to_string());
    s.append(":");
    s.append(std::to_string(e.port()));
  }
  std::hash<std::string> hasher;
  return hasher(s);
}

inline size_t endpoint_hash(asio::ip::tcp::endpoint endpoint){
  std::hash<std::string> hasher;
  return hasher(
    (std::string)endpoint.address().to_string()
    +":"
    +std::to_string(endpoint.port()));
}


class ConnHandler : public std::enable_shared_from_this<ConnHandler> {

  struct PendingRsp {
    uint32_t           id;
    DispatchHandlerPtr hdlr;
    TimerPtr           tm;
    bool               sequential;
    EventPtr           ev;
  };

  struct Outgoing {
    CommBufPtr cbuf;
    TimerPtr tm;
    DispatchHandlerPtr hdlr; 
    bool sequential;
  };

  public:
  ConnHandler(AppContextPtr app_ctx, SocketPtr socket, IOCtxPtr io_ctx) 
            : m_app_ctx(app_ctx), m_sock(socket), m_io_ctx(io_ctx), m_next_req_id(0) { }

  ConnHandlerPtr ptr(){
    return shared_from_this();
  }

  virtual ~ConnHandler(){ 
    do_close();
  }
  
  EndPoint      endpoint_remote;
  EndPoint      endpoint_local;
  AppContextPtr m_app_ctx;
  SocketPtr     m_sock;
  IOCtxPtr      m_io_ctx;

  const std::string endpoint_local_str(){
    std::string s(endpoint_local.address().to_string());
    s.append(":");
    s.append(std::to_string(endpoint_local.port()));
    return s;
  }
  
  const std::string endpoint_remote_str(){
    std::string s(endpoint_remote.address().to_string());
    s.append(":");
    s.append(std::to_string(endpoint_remote.port()));
    return s;
  }
  
  const size_t endpoint_remote_hash(){
    return endpoint_hash(endpoint_remote);
  }
  
  const size_t endpoint_local_hash(){
    return endpoint_hash(endpoint_local);
  }
  
  virtual void new_connection() {
    std::lock_guard<std::mutex> lock(m_mutex);

    endpoint_remote = m_sock->remote_endpoint();
    endpoint_local = m_sock->local_endpoint();
    HT_DEBUGF("new_connection local=%s, remote=%s, executor=%d",
              endpoint_local_str().c_str(), endpoint_remote_str().c_str(),
              (size_t)&m_sock->get_executor().context());
  }

  inline bool is_open() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_err == Error::OK && m_sock->is_open();
  }

  void close(){
    if(is_open()){
      std::lock_guard<std::mutex> lock(m_mutex);
      try{m_sock->close();}catch(...){}
    }
    m_err = Error::COMM_NOT_CONNECTED;
    disconnected();
  }

  size_t pending_read(){
    std::lock_guard<std::mutex> lock(m_mutex_reading);
    return m_pending.size();
  }

  size_t pending_write(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_outgoing.size();
  }


  virtual void run(EventPtr ev, DispatchHandlerPtr hdlr=nullptr) {
    HT_WARNF("run is Virtual!, %s", ev->to_str().c_str());
    if(hdlr != nullptr)
      hdlr->handle(ptr(), ev);
  }

  void do_close(DispatchHandlerPtr hdlr=nullptr){
    if(m_err == Error::OK) {
      close();
      run(std::make_shared<Event>(Event::Type::DISCONNECT, m_err), hdlr);
    }
  }

  int send_error(int error, const String &msg, EventPtr ev=nullptr) {
    if(m_err != Error::OK)
      return m_err;

    CommHeader header;
    if(ev != nullptr)
      header.initialize_from_request_header(ev->header);
    
    CommBufPtr cbp;
    size_t max_msg_size = std::numeric_limits<int16_t>::max();
    if (msg.length() < max_msg_size)
      cbp = Protocol::create_error_message(header, error, msg.c_str());
    else {
      cbp = Protocol::create_error_message(header, error, msg.substr(0, max_msg_size).c_str());
    }
    return send_response(cbp);
  }

  int response_ok(EventPtr ev=nullptr) {
    if(m_err != Error::OK)
      return m_err;
      
    CommHeader header;
    if(ev != nullptr)
      header.initialize_from_request_header(ev->header);
    CommBufPtr cbp = std::make_unique<CommBuf>(header, 4);
    cbp->append_i32(Error::OK);
    return send_response(cbp);
  }

  inline int send_response(CommBufPtr &cbuf, DispatchHandlerPtr hdlr=nullptr){
    if(m_err != Error::OK)
      return m_err;

    cbuf->header.flags &= CommHeader::FLAGS_MASK_REQUEST;
    cbuf->write_header_and_reset();
    
    write_or_queue({
     .cbuf=cbuf, 
     .tm=get_timer(cbuf->header.timeout_ms, cbuf->header), 
     .hdlr=hdlr, 
     .sequential=false
    });
    return m_err;
  }

  inline int send_response(CommBufPtr &cbuf, uint32_t timeout_ms){
    if(m_err != Error::OK)
      return m_err;

    int e = Error::OK;
    cbuf->header.flags &= CommHeader::FLAGS_MASK_REQUEST;
    cbuf->write_header_and_reset();

    std::future<size_t> f;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      f = asio::async_write(
        *m_sock.get(), cbuf->get_buffers(), asio::use_future);
    }
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

  /* 
  inline int send_response_sync(CommBufPtr &cbuf){
    if(m_err != Error::OK)
      return m_err;

    asio::error_code error;
    uint32_t len_sent = m_sock->write_some(cbuf->get_buffers(), error);
    return error.value() ?
           (m_err != Error::OK? m_err.load() : Error::COMM_BROKEN_CONNECTION) 
           : Error::OK;
  }
  */

  inline int send_request(uint32_t timeout_ms, CommBufPtr &cbuf, 
                    DispatchHandlerPtr hdlr, bool sequential=false) {
    cbuf->header.timeout_ms = timeout_ms;
    return send_request(cbuf, hdlr, sequential);
  }

  inline int send_request(CommBufPtr &cbuf, DispatchHandlerPtr hdlr, 
                          bool sequential=false) {
    if(m_err != Error::OK)
      return m_err;

    cbuf->header.flags |= CommHeader::FLAGS_BIT_REQUEST;

    if(cbuf->header.id == 0)
      cbuf->header.id = next_req_id();

    cbuf->write_header_and_reset();

    write_or_queue({
     .cbuf=cbuf, 
     .tm=get_timer(cbuf->header.timeout_ms, cbuf->header), 
     .hdlr=hdlr, 
     .sequential=sequential
    });
    return m_err;
  }

  inline void accept_requests() {
    read_pending();
  }

  inline void accept_requests(DispatchHandlerPtr hdlr, 
                              uint32_t timeout_ms=0) {
    add_pending({
      .id=0,  // initial req.acceptor
      .hdlr=hdlr,
      .tm=get_timer(timeout_ms)
    });
    read_pending();
  }

  private:

  inline uint32_t next_req_id(){
    return ++m_next_req_id == 0 ? ++m_next_req_id : m_next_req_id.load();
  }

  TimerPtr get_timer(uint32_t t_ms, CommHeader header=0){
    if(t_ms == 0) 
      return nullptr;

    std::lock_guard<std::mutex> lock(m_mutex);
    TimerPtr tm = std::make_shared<asio::high_resolution_timer>(
      m_sock->get_executor(), std::chrono::milliseconds(t_ms)); 

    tm->async_wait(
      [header, ptr=ptr()]
      (const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          EventPtr ev = std::make_shared<Event>(
            Event::Type::ERROR, Error::Code::REQUEST_TIMEOUT);
          ev->header = header;
          ptr->run_pending(header.id, ev, true);
        } 
      }
    );
    return tm;
  }
    
  inline void write_or_queue(Outgoing data){ 
    std::lock_guard<std::mutex> lock(m_mutex);  
    if(m_writing) {
      m_outgoing.push(data);
    } else {
      m_writing = true;
      write(data);
    }
  }
  
  inline void next_outgoing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_writing = m_outgoing.size() > 0;
    if(m_writing) {
      Outgoing data = m_outgoing.front();
      m_outgoing.pop();
      write(data); 
    }
  }
  
  inline void write(Outgoing data){

    if(data.cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      add_pending({
        .id=data.cbuf->header.id, 
        .hdlr=data.hdlr, 
        .tm=data.tm, 
        .sequential=data.sequential});

    asio::async_write(
      *m_sock.get(), 
      data.cbuf->get_buffers(),
      [ptr=ptr()](const asio::error_code ec, uint32_t len){
        if(ec)
          ptr->do_close();
        else {
          ptr->read_pending();
          ptr->next_outgoing();
        }
      }
    );
    
    
  }

  /*  do-an async_write
  void write(Outgoing data){

    //std::cout << "write id=" << data.cbuf->header.id 
    //          << " len=" << data.cbuf->header.total_len 
    //          << " data.size=" << data.cbuf->data.size 
    //          << " ext.size=" << data.cbuf->ext.size 
    //          << " " << endpoint_remote_str() << "\n";

    write_buffers(data, data.cbuf->get_buffers(0), 0);
  }

  void write_buffers(Outgoing data, std::vector<asio::const_buffer> buffers, 
                     size_t nwritten){

    m_sock->async_write_some(
      buffers,
      [data, nwritten, ptr=ptr()](const asio::error_code ec, size_t bytes_) {

        size_t written = nwritten+bytes_;






        //std::cout << "async_write_some, id=" <<  data.cbuf->header.id 
        //          << " total=" << data.cbuf->header.total_len 
        //          << " wrote=" << written
        //          << " bytes=" << bytes_ 
        //          << " ec=" << ec << "\n";
                  
        if(!ec && written < data.cbuf->header.total_len){
            std::vector<asio::const_buffer> buffs 
              = data.cbuf->get_buffers(written);
            if(buffs.size() > 0)
              ptr->write_buffers(data, buffs, written);
          return;
        }

        ptr->next_outgoing();

        if(ec)
         ptr->do_close();
        else if(data.cbuf->header.flags & CommHeader::FLAGS_BIT_REQUEST)
          ptr->read_pending({.id=data.cbuf->header.id, .hdlr=data.hdlr, 
                             .tm=data.tm, .sequential=data.sequential});
        else {    
          ptr->read_pending();
        }
      }
      //)
    );
  }
  */

  inline void add_pending(PendingRsp q){        
    std::lock_guard<std::mutex> lock(m_mutex_reading);
    m_pending.push_back(q);
  }

  inline void read_pending(){
    {
      std::lock_guard<std::mutex> lock(m_mutex_reading);
      m_pendings++;
      if(m_reading || m_err != Error::OK)
        return;
      m_reading=true;
    }

    EventPtr ev = std::make_shared<Event>(Event::Type::MESSAGE, Error::OK);
    uint8_t* data = new uint8_t[CommHeader::FIXED_LENGTH+1];

    std::lock_guard<std::mutex> lock(m_mutex);
    asio::async_read(
      *m_sock.get(), 
      asio::mutable_buffer(data, CommHeader::FIXED_LENGTH+1),
      [ev, data, ptr=ptr()](const asio::error_code e, size_t filled){
        return ptr->read_condition_hdlr(ev, data, e, filled);
      },
      [ev, ptr=ptr()](const asio::error_code e, size_t filled){
        ptr->received(ev, e, filled);
      }
    );
  }
  
  size_t read_condition_hdlr(EventPtr ev, uint8_t* data,
                             const asio::error_code e, size_t filled){
    asio::error_code ec = e;
    size_t remain = read_condition(ev, data, ec, filled);
    if(ec) {
      remain = 0;
      do_close();
    }
    if(remain == 0) 
      delete [] data;
    return remain;
  }
  
  inline size_t read_condition(EventPtr ev, uint8_t* data,
                        asio::error_code &e, size_t filled){
                  
    if(filled < CommHeader::FIXED_LENGTH)
      return CommHeader::FIXED_LENGTH-filled;
      
    try{
      ev->load_message_header(data, filled);
    } catch(...){
      ev->type = Event::Type::ERROR;
      ev->error = Error::REQUEST_TRUNCATED;
      ev->header = CommHeader();
      
      HT_WARNF("read, REQUEST HEADER_TRUNCATED: filled=%d", filled);
      e = asio::error::eof;
      return (size_t)0;
    }

    if(ev->header.total_len == ev->header.header_len)
      return (size_t)0;

    ev->payload_len = ev->header.total_len-ev->header.header_len;
    uint8_t* payload = new uint8_t[ev->payload_len]; 
    ev->payload = payload; 
    
    asio::error_code ec;
    filled = 0;
    do{
      std::lock_guard<std::mutex> lock(m_mutex);
      filled += m_sock->read_some(
        asio::mutable_buffer(payload+filled, ev->payload_len-filled),
        ec);
    } while (!ec && filled < ev->payload_len);

    if(filled != ev->payload_len) {
      ev->payload_len=0;
      e = ec;
    }
    /*
    std::cout << "read id=" << ev->header.id << " len=" << ev->header.total_len  
              << " filled=" << filled 
              << " ec=" << ec << " " << endpoint_remote_str() << "\n";
    */
    if(e){ 
      HT_WARNF("read, REQUEST PAYLOAD_TRUNCATED: filled=%d error=%s ", 
              filled, ec, ev->to_str().c_str());
    }
    return (size_t)0;
  }

  void received(EventPtr ev, const asio::error_code ec, size_t filled){
    ev->arrival_time = ClockT::now();
    if(ec) {
      do_close();
      return;
    }

    bool more;
    {
      std::lock_guard<std::mutex> lock(m_mutex_reading);
      m_reading = false;
      more = m_pendings > 0;
      if(more)
        m_pendings--;
    }
    if(more)
      read_pending();

    run_pending(ev->header.id, ev);
  }

  void disconnected(){
    {
      std::lock_guard<std::mutex> lock(m_mutex_reading);
      m_cancelled.clear();
      m_pendings = 0;
    }
    
    for(;;) {
      PendingRsp q;
      {
        std::lock_guard<std::mutex> lock(m_mutex_reading);
        if(m_pending.empty())
          return;
        q = *m_pending.begin();
        m_pending.erase(m_pending.begin());
      }
      
      if(q.tm != nullptr) q.tm->cancel();
      run(std::make_shared<Event>(Event::Type::DISCONNECT, m_err), q.hdlr);
    }
  }

  inline void run_pending(uint32_t id, EventPtr ev, bool cancelling=false){
    bool found_current = false;
    bool found_not = false;
    bool found_next;
    bool skip;
    std::vector<PendingRsp>::iterator it_found;
    std::vector<PendingRsp>::iterator it_end;

    PendingRsp q_now;
    PendingRsp q_next;
    do {
      found_next = false;
      skip = false;
      {
        std::lock_guard<std::mutex> lock(m_mutex_reading);
        /* 
        std::cout << "run_pending: m_cancelled=" << m_cancelled.size() 
                  << " m_pending=" << m_pending.size() << "\n";
        for(auto q :m_pending )
          std::cout << " id=" << q.id << "\n";
        std::cout << "looking for id=" << id << " fd="<<m_sock->native_handle()<<" ptr="<<this
        << " " << endpoint_local_str() <<"\n";
        */
        if(cancelling)
          m_cancelled.push_back(id);

        found_not = m_pending.empty();
        if(!found_not){
          it_found = std::find_if(m_pending.begin(), m_pending.end(), 
                                 [id](const PendingRsp & q){return q.id == id;});
          it_end = m_pending.end();
          found_not = it_found == it_end;
        }
        if(!found_not){
          found_current = !it_found->sequential || it_found == m_pending.begin();

          if(found_current){
            q_now = *it_found;

            if(q_now.tm != nullptr) 
              q_now.tm->cancel();

            if(q_now.sequential){
              if(++it_found != it_end){
                found_next = it_found->ev != nullptr;
                if(found_next)
                  q_next = *it_found;
              }
              m_pending.erase(--it_found);
            } else
              m_pending.erase(it_found);

          } else {
            it_found->ev=ev;
          }
          
        } else if(!cancelling) {
            auto it = std::find_if(m_cancelled.begin(), m_cancelled.end(), 
                                  [id](uint32_t i){return i == id;});
            skip = (it != m_cancelled.end());
            if(skip)
              m_cancelled.erase(it);
        }
      }
    
      if(found_not){
        if(id == 0)
          run(ev);
        else if(!skip)
          run_pending(0, ev);
        
      } else if(found_current){

        run(ev, q_now.hdlr);
        
        if(found_next)
          run_pending(q_next.id, q_next.ev);
      }
    } while(found_next);

  }

  std::mutex                m_mutex;
  std::queue<Outgoing>      m_outgoing;
  bool                      m_writing = 0;

  std::mutex                m_mutex_reading;
  std::vector<PendingRsp>   m_pending;
  std::vector<uint32_t>     m_cancelled;
  uint32_t                  m_pendings = 0;
  bool                      m_reading = 0;

  std::atomic<Error::Code>  m_err = Error::OK;
  std::atomic<uint32_t>     m_next_req_id;

};


}

#endif // swc_core_comm_ConnHandler_h