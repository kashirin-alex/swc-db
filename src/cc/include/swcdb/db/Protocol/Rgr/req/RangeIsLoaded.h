
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_RangeIsLoaded_h
#define swc_lib_db_protocol_rgr_req_RangeIsLoaded_h

#include "swcdb/db/Protocol/Rgr/params/RangeIsLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeIsLoaded : public DispatchHandler {
  public:
  
  typedef std::function<void(bool)> RangeIsLoaded_t;

  RangeIsLoaded(ConnHandlerPtr conn, DB::RangeBase::Ptr range, 
                RangeIsLoaded_t cb)
                : conn(conn), range(range), cb(cb), was_called(false) { }
  
  virtual ~RangeIsLoaded() { }
  
  bool run(uint32_t timeout=60000) override {
    auto cbp = CommBuf::make(Params::RangeIsLoaded(range->cfg->cid, range->rid));
    cbp->header.set(RANGE_IS_LOADED, timeout);
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void disconnected() {};

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    
    // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        cb(false);
      return;
    }

    if(ev->header.command == RANGE_IS_LOADED){
      was_called = true;
      cb(ev->response_code() == Error::OK);
    }

  }

  private:
  ConnHandlerPtr            conn;
  DB::RangeBase::Ptr        range;
  RangeIsLoaded_t           cb;
  std::atomic<bool>         was_called;
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_RangeIsLoaded_h
