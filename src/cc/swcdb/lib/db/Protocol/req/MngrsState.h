
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrsState_h
#define swc_lib_db_protocol_req_MngrsState_h

#include "swcdb/lib/db/Protocol/req/ConnQueue.h"
#include "swcdb/lib/db/Protocol/params/MngrsState.h"
 
namespace SWC {
namespace Protocol {
namespace Req {

class MngrsState : public ConnQueue::ReqBase {
  public:

  MngrsState(ResponseCallbackPtr cb, server::Mngr::MngrsStatus &states, 
              uint64_t token, const EndPoint& mngr_host, uint32_t timeout) 
            : cb(cb) {
    Protocol::Params::MngrsState params(states, token, mngr_host);
    CommHeader header(Protocol::Command::MNGR_REQ_MNGRS_STATE, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrsState() { }

  void disconnected(ConnHandlerPtr conn);

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called)
      return;

    if(ev->type == Event::Type::DISCONNECT){
      disconnected(conn);
      return;
    }
    if(ConnQueue::ReqBase::is_timeout(conn, ev))
      return;

    if(ev->header.command == Protocol::Command::MNGR_REQ_MNGRS_STATE 
       && Protocol::response_code(ev) == Error::OK){
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
  ResponseCallbackPtr   cb;
};

}}}

#endif // swc_lib_db_protocol_req_MngrsState_h
