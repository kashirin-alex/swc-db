
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/db/Protocol/Common/params/ColRangeId.h"
#include "swcdb/db/Protocol/Commands.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


RangeUnload::RangeUnload(const Ranger::RangePtr& range, 
                         const ResponseCallback::Ptr& cb,
                         uint32_t timeout) 
                        : client::ConnQueue::ReqBase(false), 
                          cb(cb), range(range) {
  cbp = CommBuf::make(Common::Params::ColRangeId(range->cfg->cid, range->rid));
  cbp->header.set(RANGE_UNLOAD, timeout);
}

RangeUnload::~RangeUnload() { }

void RangeUnload::handle(ConnHandlerPtr, const Event::Ptr&) {
  unloaded(valid() ? Error::OK : Error::RGR_DELETED_RANGE);
}

bool RangeUnload::valid() {
  return !range->deleted();
}

void RangeUnload::handle_no_conn() {
  unloaded(Error::OK); 
}

void RangeUnload::unloaded(int err) {
  range->take_ownership(err, cb);
}

}}}}
