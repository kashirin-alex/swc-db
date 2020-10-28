
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


RangeUnload::RangeUnload(const Manager::Ranger::Ptr& rgr,
                         const Manager::Column::Ptr& col, 
                         const Manager::Range::Ptr& range,
                         bool ignore_error,
                         uint32_t timeout) 
        : client::ConnQueue::ReqBase(
            false,
            Buffers::make(
              Common::Params::ColRangeId(range->cfg->cid, range->rid),
              0,
              RANGE_UNLOAD, timeout
            )
          ), 
          rgr(rgr), col(col), range(range), ignore_error(ignore_error) {
}
  
RangeUnload::~RangeUnload() { }

bool RangeUnload::valid() {
  return col->state() != DB::Types::MngrColumn::State::DELETED &&
         !range->deleted();
}
  
void RangeUnload::handle_no_conn() {
  unloaded(Error::COMM_NOT_CONNECTED);
}

void RangeUnload::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(!valid())
    return;

  unloaded(ev->response_code());
}

  
void RangeUnload::unloaded(int err) {
  if(err) {
    if(!ignore_error)
      ++rgr->failures;
  } else if(range->get_rgr_id() == rgr->rgrid) {
    col->set_unloaded(range);
    Env::Mngr::rangers()->schedule_check(2);
  }
}


}}}}}
