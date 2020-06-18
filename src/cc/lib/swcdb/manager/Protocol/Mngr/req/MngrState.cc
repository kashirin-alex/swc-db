
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 


#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


MngrState::MngrState(const ResponseCallback::Ptr& cb, 
                     const Manager::MngrsStatus& states, 
                     uint64_t token, const EndPoint& mngr_host, 
                     uint32_t timeout) 
                    : cb(cb) {
  cbp = CommBuf::make(Params::MngrState(states, token, mngr_host));
  cbp->header.set(MNGR_STATE, timeout);
}
  
MngrState::~MngrState() { }

void MngrState::disconnected(const ConnHandlerPtr& conn) {
  Env::Mngr::role()->disconnection(
    conn->endpoint_remote, conn->endpoint_local);
}

void MngrState::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {
  if(was_called)
    return;

  if(ev->type == Event::Type::DISCONNECT){
    disconnected(conn);
    return;
  }
  if(client::ConnQueue::ReqBase::is_timeout(conn, ev))
    return;

  if(ev->header.command == MNGR_STATE && ev->response_code() == Error::OK){
    if(cb != nullptr){
      //std::cout << "response_ok, cb=" << (size_t)cb.get() 
      //          << " rsp, err=" << ev->to_str() << "\n";
      cb->response_ok();
    }
    was_called = true;
    return;
  }

  conn->do_close();
}

}}}}


#include "swcdb/manager/Protocol/Mngr/params/MngrState.cc"