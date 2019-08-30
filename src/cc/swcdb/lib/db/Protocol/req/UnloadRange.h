
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

  UnloadRange(client::ClientConPtr conn, DB::RangeBasePtr range, bool sync=true)
            : conn(conn), range(range), sync(sync) { 
    if(sync)
      result_future = result_promise.get_future();
  }
  
  virtual ~UnloadRange() { }
  
  bool run(uint32_t timeout=60000) override {
    Protocol::Params::ColRangeId params = 
      Protocol::Params::ColRangeId(range->cid, range->rid);
    
    CommHeader header(Protocol::Command::REQ_RS_UNLOAD_RANGE, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
    
    bool ok = conn->send_request(cbp, shared_from_this()) == Error::OK;
    if(sync)
      result_future.get();
    return ok;
  }

  void disconnected() {};

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT)
      return;
    
    conn->do_close();

    result_promise.set_value(true);

    if(ev->header.command == Protocol::Command::REQ_RS_UNLOAD_RANGE 
       && Protocol::response_code(ev) == Error::OK){
      HT_DEBUGF("RANGE-UNLOADED %s", range->to_string().c_str());
      return;
    }

  }

  private:
  client::ClientConPtr  conn;
  DB::RangeBasePtr      range;

  bool sync;
  std::promise<bool>  result_promise;
  std::future<bool>   result_future;
};

}}}

#endif // swc_lib_db_protocol_req_UnloadRange_h
