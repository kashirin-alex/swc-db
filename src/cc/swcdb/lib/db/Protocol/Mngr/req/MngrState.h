
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_mngr_req_MngrState_h
#define swc_lib_db_protocol_mngr_req_MngrState_h

#include "../params/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


class MngrState : public Common::Req::ConnQueue::ReqBase {
  public:

  MngrState(ResponseCallbackPtr cb, server::Mngr::MngrsStatus &states, 
            uint64_t token, const EndPoint& mngr_host, uint32_t timeout) 
            : cb(cb) {
    Params::MngrState params(states, token, mngr_host);
    CommHeader header(MNGR_STATE, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrState() { }

  void disconnected(ConnHandlerPtr conn);

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called)
      return;

    if(ev->type == Event::Type::DISCONNECT){
      disconnected(conn);
      return;
    }
    if(Common::Req::ConnQueue::ReqBase::is_timeout(conn, ev))
      return;

    if(ev->header.command == MNGR_STATE && response_code(ev) == Error::OK){
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

}}}}

#endif // swc_lib_db_protocol_mngr_req_MngrState_h
