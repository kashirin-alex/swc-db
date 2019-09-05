
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsColumnDelete_h
#define swc_lib_db_protocol_req_RsColumnDelete_h

#include "Callbacks.h"
#include "swcdb/lib/db/Protocol/params/ColumnId.h"

namespace SWC {
namespace Protocol {
namespace Req {

class RsColumnDelete : public DispatchHandler {
  public:

  RsColumnDelete(client::ClientConPtr conn, int64_t cid, 
            Callback::RsColumnDelete_t cb)
            : conn(conn), cid(cid), cb(cb), was_called(false) { }
  
  virtual ~RsColumnDelete() { }
  
  bool run(uint32_t timeout=60000) override {
    Protocol::Params::ColumnId params = Protocol::Params::ColumnId(cid);
    
    CommHeader header(Protocol::Command::REQ_RS_COLUMN_DELETE, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn_ptr, EventPtr &ev) {
      
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(Error::COMM_NOT_CONNECTED);
      return;
    }

    if(ev->header.command == Protocol::Command::REQ_RS_COLUMN_DELETE){
      was_called = true;
      cb(Protocol::response_code(ev));
    }
    
  }

  private:
  client::ClientConPtr  conn;
  int64_t               cid;
  Callback::RsColumnDelete_t cb;
  std::atomic<bool>     was_called;
   
};

}}}

#endif // swc_lib_db_protocol_req_RsColumnDelete_h
