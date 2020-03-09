
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/db/Protocol/Common/params/ColRangeId.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


RangeUnload::RangeUnload(Ranger::RangePtr range, ResponseCallback::Ptr cb,
                         uint32_t timeout) 
                        : client::ConnQueue::ReqBase(false), 
                          range(range), cb(cb) {
  cbp = CommBuf::make(Common::Params::ColRangeId(range->cfg->cid, range->rid));
  cbp->header.set(RANGE_UNLOAD, timeout);
}

RangeUnload::~RangeUnload() { }

void RangeUnload::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
      
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


bool RangeUnload::valid() {
  return !range->deleted();
}

void RangeUnload::handle_no_conn() {
  unloaded(Error::OK, cb); 
}

void RangeUnload::unloaded(int err, ResponseCallback::Ptr cb) {
  range->take_ownership(err, cb);
}

}}}}
