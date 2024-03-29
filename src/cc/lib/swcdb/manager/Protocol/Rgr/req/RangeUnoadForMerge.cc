/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
RangeUnoadForMerge::RangeUnoadForMerge(
  const Manager::Ranger::Ptr& a_rgr,
  const Manager::ColumnHealthCheck::ColumnMerger::RangesMerger::Ptr& a_merger,
  const Manager::Range::Ptr& a_range,
  uint32_t timeout)
        : client::ConnQueue::ReqBase(
            Buffers::make(
              Params::RangeUnload(
                a_range->cfg->cid, a_range->rid,
                Params::RangeUnload::Flag::CHECK_EMPTY),
              0,
              RANGE_UNLOAD, timeout
            )
          ),
          rgr(a_rgr), merger(a_merger), range(a_range) {
}

bool RangeUnoadForMerge::valid() {
  return merger->col_merger->col_checker->col->state()
          != DB::Types::MngrColumn::State::DELETED &&
         !range->deleted() &&
         Env::Mngr::rangers()->running();
}

void RangeUnoadForMerge::handle_no_conn() {
  merger->handle(range, Error::COMM_NOT_CONNECTED, false);
}

void RangeUnoadForMerge::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::RangeUnloadRsp rsp_params(
    valid()
      ? ev->error
      : (Env::Mngr::rangers()->running()
          ? Error::COLUMN_MARKED_REMOVED
          : Error::SERVER_SHUTTING_DOWN));
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
  merger->handle(
    range, rsp_params.err, rsp_params.flags & Params::RangeUnloadRsp::EMPTY);
}



}}}}}
