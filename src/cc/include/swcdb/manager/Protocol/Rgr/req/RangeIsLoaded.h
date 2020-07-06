
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_RangeIsLoaded_h
#define swc_manager_Protocol_rgr_req_RangeIsLoaded_h

#include "swcdb/db/Protocol/Rgr/params/RangeIsLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeIsLoaded : public DispatchHandler {
  public:
  
  typedef std::function<void(bool)> RangeIsLoaded_t;

  RangeIsLoaded(const ConnHandlerPtr& conn, 
                const Manager::Range::Ptr& range,
                const RangeIsLoaded_t& cb,
                uint32_t timeout=60000)
                : conn(conn), range(range), cb(cb), timeout(timeout), 
                  was_called(false) { 
  }
  
  virtual ~RangeIsLoaded() { }
  
  bool run() override {
    auto cbp = CommBuf::make(
      Params::RangeIsLoaded(range->cfg->cid, range->rid));
    cbp->header.set(RANGE_IS_LOADED, timeout);
    return conn->send_request(cbp, shared_from_this());
  }

  void disconnected() {};

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    
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
  Manager::Range::Ptr       range;
  RangeIsLoaded_t           cb;
  uint32_t                  timeout;
  std::atomic<bool>         was_called;
};

}}}}

#endif // swc_manager_Protocol_rgr_req_RangeIsLoaded_h
