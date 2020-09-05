
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


MngrState::MngrState(const ResponseCallback::Ptr& cb, 
                     const Manager::MngrsStatus& states, 
                     uint64_t token, const EndPoint& mngr_host, 
                     uint32_t timeout) 
                    : client::ConnQueue::ReqBase(true), cb(cb) {
  cbp = CommBuf::make(Params::MngrState(states, token, mngr_host));
  cbp->header.set(MNGR_STATE, timeout);
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