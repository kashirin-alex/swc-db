
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_UnloadRange_h
#define swc_lib_db_protocol_req_UnloadRange_h

#include "swcdb/lib/db/Protocol/params/ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Req {

class UnloadRange : public DispatchHandler {
  public:

  UnloadRange(client::ClientConPtr conn, RangePtr range, bool sync=true)
            : conn(conn), range(range) { 
    if(sync){

    }
  }
  
  virtual ~UnloadRange() { }
  
  bool run(uint32_t timeout=60000) override {
    Protocol::Params::ColRangeId params = 
      Protocol::Params::ColRangeId(range->cid, range->rid);
    
    CommHeader header(Protocol::Command::MNGR_REQ_LOAD_RANGE, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void disconnected() {};

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT){
      disconnected();
      return;
    }

    if(ev->header.command == Protocol::Command::MNGR_REQ_LOAD_RANGE 
       && Protocol::response_code(ev) == Error::OK){
      range->set_loaded(true);
      std::cout << "Req, RANGE-LOADED, cid=" << range->cid << " rid=" << range->rid << "\n";
      return;
    }

    conn->do_close();

  }

  private:
  client::ClientConPtr  conn;
  RangePtr              range;
  ;
};

typedef std::shared_ptr<UnloadRange> UnloadRangePtr;

}}}

#endif // swc_lib_db_protocol_req_UnloadRange_h
