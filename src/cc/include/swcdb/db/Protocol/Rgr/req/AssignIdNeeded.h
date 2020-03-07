
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h
#define swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class AssignIdNeeded : public client::ConnQueue::ReqBase {
  public:

  AssignIdNeeded(server::Mngr::Ranger::Ptr rs_chk, 
                 server::Mngr::Ranger::Ptr rs_nxt, 
                 server::Mngr::Range::Ptr range) 
                : client::ConnQueue::ReqBase(false), 
                  rs_chk(rs_chk), rs_nxt(rs_nxt), range(range) {
    cbp = CommBuf::make();
    cbp->header.set(ASSIGN_ID_NEEDED, 60000);
  }
  
  virtual ~AssignIdNeeded() { }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == ASSIGN_ID_NEEDED){
      rsp(ev->error != Error::OK ? ev->error : ev->response_code());
      return;
    }
  }

  bool valid() override {
    return !range->deleted();
  }

  void handle_no_conn() override {
    rsp(Error::COMM_NOT_CONNECTED);
  };

  void rsp(int err);


  server::Mngr::Ranger::Ptr rs_nxt;
  server::Mngr::Range::Ptr  range;
  private:

  server::Mngr::Ranger::Ptr rs_chk;
};

}}}}

#endif // swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h
