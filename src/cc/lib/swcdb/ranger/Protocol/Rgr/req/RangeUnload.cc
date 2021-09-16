/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
RangeUnload::RangeUnload(const Ranger::RangePtr& a_range,
                         const Ranger::Callback::RangeLoad::Ptr& a_req,
                         uint32_t timeout)
        : client::ConnQueue::ReqBase(
            Buffers::make(
              Params::RangeUnload(a_range->cfg->cid, a_range->rid),
              0,
              RANGE_UNLOAD, timeout)
          ),
          req(a_req), range(a_range) {
}

void RangeUnload::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::RangeUnloadRsp rsp_params(valid() ? Error::OK : Error::RGR_DELETED_RANGE);
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

bool RangeUnload::valid() {
  return !range->deleted() &&
         !range->state_unloading();
}

void RangeUnload::handle_no_conn() {
  unloaded(Error::OK);
}

void RangeUnload::unloaded(int err) {
  range->internal_take_ownership(err, req);
}

}}}}}
