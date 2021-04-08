/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


RangeUnload::RangeUnload(const Ranger::RangePtr& range,
                         const Ranger::Callback::RangeLoad::Ptr& req,
                         uint32_t timeout)
        : client::ConnQueue::ReqBase(
            false,
            Buffers::make(
              Params::RangeUnload(range->cfg->cid, range->rid),
              0,
              RANGE_UNLOAD, timeout)
          ),
          req(req), range(range) {
}

RangeUnload::~RangeUnload() { }

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
  return !range->deleted();
}

void RangeUnload::handle_no_conn() {
  unloaded(Error::OK);
}

void RangeUnload::unloaded(int err) {
  range->internal_take_ownership(err, req);
}

}}}}}
