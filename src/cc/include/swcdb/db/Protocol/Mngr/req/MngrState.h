
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_mngr_req_MngrState_h
#define swc_lib_db_protocol_mngr_req_MngrState_h

#include "swcdb/db/Protocol/Mngr/params/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


class MngrState : public client::ConnQueue::ReqBase {
  public:

  MngrState(ResponseCallback::Ptr cb, server::Mngr::MngrsStatus &states, 
            uint64_t token, const EndPoint& mngr_host, uint32_t timeout) 
            : cb(cb) {
    cbp = CommBuf::make(Params::MngrState(states, token, mngr_host));
    cbp->header.set(MNGR_STATE, timeout);
  }
  
  virtual ~MngrState() { }

  void disconnected(ConnHandlerPtr conn);

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
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

  private:
  ResponseCallback::Ptr   cb;
};

}}}}

#endif // swc_lib_db_protocol_mngr_req_MngrState_h
