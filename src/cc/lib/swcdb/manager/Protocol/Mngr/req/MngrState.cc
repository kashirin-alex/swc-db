
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


MngrState::MngrState(const Comm::ResponseCallback::Ptr& cb, 
                     const Manager::MngrsStatus& states, 
                     uint64_t token, const Comm::EndPoint& mngr_host, 
                     uint32_t timeout) 
                    : Comm::client::ConnQueue::ReqBase(true), cb(cb) {
  cbp = Comm::Buffers::make(Params::MngrState(states, token, mngr_host));
  cbp->header.set(MNGR_STATE, timeout);
}
  
MngrState::~MngrState() { }

void MngrState::disconnected(const Comm::ConnHandlerPtr&) {
  //Env::Mngr::role()->disconnection(
  //  conn->endpoint_remote, conn->endpoint_local);
}

void MngrState::handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) {

  if(ev->type == Comm::Event::Type::DISCONNECT) {
    //disconnected(conn);
    return;
  }
  if(Comm::client::ConnQueue::ReqBase::is_timeout(ev))
    return;

  if(ev->response_code() == Error::OK) {
    if(cb != nullptr){
      //SWC_PRINT << "response_ok, cb=" << (size_t)cb.get() 
      //          << " rsp, err=" << ev->to_str() << SWC_PRINT_CLOSE;
      cb->response_ok();
    }
  } else {
    conn->do_close();
  }
}

}}}}


#include "swcdb/manager/Protocol/Mngr/params/MngrState.cc"