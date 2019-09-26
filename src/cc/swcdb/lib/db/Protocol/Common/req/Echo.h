
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Echo_h
#define swc_lib_db_protocol_common_req_Echo_h


namespace SWC { namespace Protocol { namespace Common { namespace Req {


class Echo : public DispatchHandler {
  public:
  typedef std::function<void(bool)> EchoCb_t;

  Echo(client::ClientConPtr conn, EchoCb_t cb)
       : conn(conn), cb(cb), was_called(false) { }
  
  virtual ~Echo() { }
  
  bool run(uint32_t timeout=60000) override {
    CommHeader header(DO_ECHO, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header);
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn_ptr, EventPtr &ev) {
      
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
  client::ClientConPtr  conn;
  EchoCb_t              cb;
  std::atomic<bool>     was_called;
   
};

}}}}

#endif // swc_lib_db_protocol_common_req_Echo_h
