/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
RangeUnload::RangeUnload(const Manager::Ranger::Ptr& rgr,
                         const Manager::Column::Ptr& col,
                         const Manager::Range::Ptr& range,
                         bool ignore_error,
                         uint32_t timeout)
        : client::ConnQueue::ReqBase(
            Buffers::make(
              Params::RangeUnload(range->cfg->cid, range->rid),
              0,
              RANGE_UNLOAD, timeout
            )
          ),
          rgr(rgr), col(col), range(range), ignore_error(ignore_error) {
}

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

  Params::RangeUnloadRsp rsp_params(ev->error);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }
  unloaded(rsp_params.err);
}


void RangeUnload::unloaded(int err) {
  if(err) {
    if(!ignore_error)
      rgr->failures.fetch_add(1);
  } else if(range->get_rgr_id() == rgr->rgrid) {
    col->set_unloaded(range);
    Env::Mngr::rangers()->schedule_check(2);
  }
}


}}}}}
