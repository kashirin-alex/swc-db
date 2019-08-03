
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsIdReqNeeded_h
#define swc_lib_db_protocol_req_RsIdReqNeeded_h

#include "Callbacks.h"

namespace SWC {
namespace Protocol {
namespace Req {

class RsIdReqNeeded : public DispatchHandler {
  public:

  RsIdReqNeeded(client::ClientConPtr conn, Callback::RsIdReqNeeded_t cb)
                : conn(conn), cb(cb), was_called(false) { }
  
  virtual ~RsIdReqNeeded() { }
  
  bool run(uint32_t timeout=60000) override {
    CommHeader header(Protocol::Command::REQ_RS_ASSIGN_ID_NEEDED, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, 0);
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn_ptr, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(false);
      return;
    }

    if(ev->header.command == Protocol::Command::REQ_RS_ASSIGN_ID_NEEDED){
      was_called = true;
      cb(Protocol::response_code(ev) == Error::OK);
    }

  }

  private:
  client::ClientConPtr      conn;
  Callback::RsIdReqNeeded_t cb;
  std::atomic<bool>         was_called;
};

}}}

#endif // swc_lib_db_protocol_req_RsIdReqNeeded_h
