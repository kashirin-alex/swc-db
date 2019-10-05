
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h
#define swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class AssignIdNeeded : public Common::Req::ConnQueue::ReqBase {
  public:

  AssignIdNeeded(server::Mngr::RangerPtr rs_chk, 
                 server::Mngr::RangerPtr rs_nxt, 
                 server::Mngr::Range::Ptr range) 
                : Common::Req::ConnQueue::ReqBase(false), 
                  rs_chk(rs_chk), rs_nxt(rs_nxt), range(range) {
    CommHeader header(ASSIGN_ID_NEEDED, 60000);
    cbp = std::make_shared<CommBuf>(header, 0);
  }
  
  virtual ~AssignIdNeeded() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == ASSIGN_ID_NEEDED){
      rsp(ev->error != Error::OK ? ev->error : response_code(ev));
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


  server::Mngr::RangerPtr   rs_nxt;
  server::Mngr::Range::Ptr  range;
  private:

  server::Mngr::RangerPtr  rs_chk;
};

}}}}

#endif // swc_lib_db_protocol_Rgr_req_AssignIdNeeded_h
