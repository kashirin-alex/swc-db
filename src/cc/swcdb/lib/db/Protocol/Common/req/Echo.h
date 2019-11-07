
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Echo_h
#define swc_lib_db_protocol_common_req_Echo_h


namespace SWC { namespace Protocol { namespace Common { namespace Req {


class Echo : public DispatchHandler {
  public:
  typedef std::function<void(bool)> EchoCb_t;

  Echo(client::ConnHandler::Ptr conn, EchoCb_t cb, size_t buf_sz=0)
       : conn(conn), cb(cb), was_called(false) { 

    if(!buf_sz) {
      cbp = CommBuf::make();

    } else {
      StaticBuffer sndbuf(buf_sz);
      uint8_t* ptr = sndbuf.base;
      const uint8_t* end = sndbuf.base + buf_sz-4;
    
      uint8_t i=0;
      while(ptr < end) {
        if(i == 127)
          i = 0;
        else
          i++;
        *ptr++ = i;
      }
      
      cbp = CommBuf::make(sndbuf);
    }
    
    cbp->header.set(DO_ECHO, 60000);
  }
  
  virtual ~Echo() { }
  
  bool run(uint32_t timeout=60000) override {
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn_ptr, Event::Ptr &ev) {
      
    //HT_DEBUGF("handle: %s", ev->to_str().c_str());

    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(false);
      return;
    }

    if(ev->header.command == DO_ECHO){
      was_called = true;
      cb(ev->error == Error::OK);
    }
  }

  private:
  client::ConnHandler::Ptr  conn;
  EchoCb_t              cb;
  std::atomic<bool>     was_called;
  CommBuf::Ptr          cbp;
};

}}}}

#endif // swc_lib_db_protocol_common_req_Echo_h
