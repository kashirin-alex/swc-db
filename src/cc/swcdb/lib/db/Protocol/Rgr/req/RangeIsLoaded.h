
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_RangeIsLoaded_h
#define swc_lib_db_protocol_rgr_req_RangeIsLoaded_h

#include "../params/RangeIsLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeIsLoaded : public DispatchHandler {
  public:
  
  typedef std::function<void(bool)> RangeIsLoaded_t;

  RangeIsLoaded(client::ClientConPtr conn, DB::RangeBase::Ptr range, 
                RangeIsLoaded_t cb)
                : conn(conn), range(range), cb(cb), was_called(false) { }
  
  virtual ~RangeIsLoaded() { }
  
  bool run(uint32_t timeout=60000) override {
    Params::RangeIsLoaded params(range->cid, range->rid);
    CommHeader header(RANGE_IS_LOADED, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void disconnected() {};

  void handle(ConnHandlerPtr conn_ptr, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(false);
      return;
    }

    if(ev->header.command == RANGE_IS_LOADED){
      was_called = true;
      cb(response_code(ev) == Error::OK);
    }

  }

  private:
  client::ClientConPtr      conn;
  DB::RangeBase::Ptr        range;
  RangeIsLoaded_t           cb;
  std::atomic<bool>         was_called;
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_RangeIsLoaded_h
