
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_RangeUnload_h
#define swc_lib_db_protocol_rgr_req_RangeUnload_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeUnload : public Common::Req::ConnQueue::ReqBase {
  public:

  RangeUnload(DB::RangeBase::Ptr range, ResponseCallback::Ptr cb,
                uint32_t timeout=60000) 
               : Common::Req::ConnQueue::ReqBase(false), 
                range(range), cb(cb) {
    cbp = CommBuf::make(Common::Params::ColRangeId(range->cfg->cid, range->rid));
    cbp->header.set(RANGE_UNLOAD, timeout);
  }

  virtual ~RangeUnload() { }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid())
      unloaded(Error::RS_DELETED_RANGE, cb); 
    else if(ev->type == Event::Type::DISCONNECT
            || ev->header.command == RANGE_UNLOAD){
      unloaded(Error::OK, cb); 
    }
  }

  bool valid() override;
  
  void handle_no_conn() override {
    unloaded(Error::OK, cb); 
  }

  void unloaded(int err, ResponseCallback::Ptr cb);


  private:

  ResponseCallback::Ptr cb;
  DB::RangeBase::Ptr    range;
   
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_RangeUnload_h
