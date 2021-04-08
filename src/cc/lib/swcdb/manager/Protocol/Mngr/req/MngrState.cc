/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr {namespace Req {


MngrState::MngrState(const ResponseCallback::Ptr& cb,
                     const Manager::MngrsStatus& states,
                     uint64_t token,
                     const EndPoint& mngr_host,
                     uint32_t timeout)
            : client::ConnQueue::ReqBase(
                true,
                Buffers::make(
                  Params::MngrState(states, token, mngr_host),
                  0,
                  MNGR_STATE, timeout
                )
              ),
              cb(cb) {
}

MngrState::~MngrState() { }

void MngrState::disconnected(const ConnHandlerPtr&) {
  //Env::Mngr::role()->disconnection(
  //  conn->endpoint_remote, conn->endpoint_local);
}

void MngrState::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT) {
    //disconnected(conn);
    return;
  }
  if(client::ConnQueue::ReqBase::is_timeout(ev))
    return;

  if(ev->response_code() == Error::OK) {
    if(cb) {
      //SWC_PRINT << "response_ok, cb=" << size_t(cb.get())
      //          << " rsp, err=" << ev->to_str() << SWC_PRINT_CLOSE;
      cb->response_ok();
    }
  } else {
    conn->do_close();
  }
}

}}}}}


#include "swcdb/manager/Protocol/Mngr/params/MngrState.cc"
