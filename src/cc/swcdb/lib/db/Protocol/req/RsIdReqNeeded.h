
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsIdReqNeeded_h
#define swc_lib_db_protocol_req_RsIdReqNeeded_h

namespace SWC { namespace Protocol { namespace Req {
  

class RsIdReqNeeded : public ConnQueue::ReqBase {
  public:

  RsIdReqNeeded(server::Mngr::RsStatusPtr rs_chk, 
                server::Mngr::RsStatusPtr rs_nxt, 
                server::Mngr::RangePtr range) : ConnQueue::ReqBase(false), 
                rs_chk(rs_chk), rs_nxt(rs_nxt), range(range) {
    CommHeader header(Command::REQ_RS_ASSIGN_ID_NEEDED, 60000);
    cbp = std::make_shared<CommBuf>(header, 0);
  }
  
  virtual ~RsIdReqNeeded() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == Command::REQ_RS_ASSIGN_ID_NEEDED){
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


  server::Mngr::RsStatusPtr rs_nxt;
  server::Mngr::RangePtr    range;
  private:

  server::Mngr::RsStatusPtr rs_chk;
};

}}}

#endif // swc_lib_db_protocol_req_RsIdReqNeeded_h
