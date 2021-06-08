/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr {namespace Req {


SWC_SHOULD_INLINE
MngrState::MngrState(const ResponseCallback::Ptr& cb,
                     const Manager::MngrsStatus& states,
                     uint64_t token,
                     const EndPoint& mngr_host,
                     uint32_t timeout)
            : client::ConnQueue::ReqBase(
                Buffers::make(
                  Params::MngrState(states, token, mngr_host),
                  0,
                  MNGR_STATE, timeout
                )
              ),
              cb(cb) {
}

void MngrState::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {
  if(!ev->response_code()) {
    if(cb)
      cb->response_ok();
  } else {
    conn->do_close();
  }
}

}}}}}


#include "swcdb/manager/Protocol/Mngr/params/MngrState.cc"
