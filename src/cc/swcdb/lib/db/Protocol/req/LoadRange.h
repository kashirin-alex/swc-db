
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_LoadRange_h
#define swc_lib_db_protocol_req_LoadRange_h

#include "Callbacks.h"
#include "swcdb/lib/db/Protocol/params/ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Req {

class LoadRange : public DispatchHandler {
  public:

  LoadRange(client::ClientConPtr conn, DB::RangeBasePtr range, 
            Callback::LoadRange_t cb)
            : conn(conn), range(range), cb(cb), was_called(false) { }
  
  virtual ~LoadRange() { }
  
  bool run(uint32_t timeout=60000) override {
    Protocol::Params::ColRangeId params = 
      Protocol::Params::ColRangeId(range->cid, range->rid);
    
    CommHeader header(Protocol::Command::REQ_RS_LOAD_RANGE, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn_ptr, EventPtr &ev) {
      
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(false);
      return;
    }

    if(ev->header.command == Protocol::Command::REQ_RS_LOAD_RANGE){
      was_called = true;
      cb(Protocol::response_code(ev) == Error::OK);
    }
    
  }

  private:
  client::ClientConPtr  conn;
  DB::RangeBasePtr      range;
  Callback::LoadRange_t cb;
  std::atomic<bool>     was_called;
   
};

}}}

#endif // swc_lib_db_protocol_req_LoadRange_h
