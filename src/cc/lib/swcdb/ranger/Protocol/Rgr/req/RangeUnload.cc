
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/db/Protocol/Common/params/ColRangeId.h"
#include "swcdb/db/Protocol/Commands.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


RangeUnload::RangeUnload(const Ranger::RangePtr& range, 
                         const Comm::ResponseCallback::Ptr& cb,
                         uint32_t timeout) 
                        : Comm::client::ConnQueue::ReqBase(false), 
                          cb(cb), range(range) {
  cbp = Comm::CommBuf::make(Common::Params::ColRangeId(range->cfg->cid, range->rid));
  cbp->header.set(RANGE_UNLOAD, timeout);
}

RangeUnload::~RangeUnload() { }

void RangeUnload::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr&) {
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
