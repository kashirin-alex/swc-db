
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngrsState_h
#define swc_lib_db_protocol_req_MngrsState_h



namespace SWC {
namespace Protocol {
namespace Req {

class MngrsState : public DispatchHandler {
  public:

  MngrsState(client::ClientConPtr conn, 
             server::Mngr::HostStatuses states, uint64_t token,
             ResponseCallbackPtr cb, 
             server::Mngr::RoleStatePtr role_state)
            : conn(conn), states(states), token(token), cb(cb), 
              role_state(role_state) { }
  
  virtual ~MngrsState(){

    HT_INFOF("destruct: cb=%d, token=%d", 
              (size_t)cb.get(), token);
  }
  
  bool run(uint32_t timeout=60000) override {
    std::cout << " token=" << token << " cb=" << (size_t)cb.get() << " req\n";

    Protocol::Params::MngrsState params(states, token);
    CommHeader header(Protocol::Command::MNGR_REQ_MNGRS_STATE, timeout);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void disconnected();

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    std::cout << " token=" << token << " cb=" << (size_t)cb.get() << " rsp, err=" << ev->error << "\n";

    //HT_INFOF("handle: %s", ev->to_str().c_str());
    if(ev->type == Event::Type::DISCONNECT){
      disconnected();
      return;
    }

    if(ev->header.command == Protocol::Command::MNGR_REQ_MNGRS_STATE) {
      if(cb != nullptr)
        cb->response_ok();
      return;
    }

    if(ev->error != Error::OK){
      if(ev->error == Error::Code::REQUEST_TIMEOUT){
        //if(--probes > 1 && run())
        //  return;
        conn->do_close();
        return;
      }
      HT_INFOF("unhandled error: %s", ev->to_str().c_str());
    } 

  }

  private:
  server::Mngr::RoleStatePtr role_state;
  ResponseCallbackPtr cb;
  client::ClientConPtr conn;
  server::Mngr::HostStatuses states;
  uint64_t token;
  int probes = 3;
};

typedef std::shared_ptr<MngrsState> MngrsStatePtr;

}}}

#endif // swc_lib_db_protocol_req_MngrsState_h
