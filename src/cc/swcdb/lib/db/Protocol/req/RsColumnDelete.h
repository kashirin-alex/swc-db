
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsColumnDelete_h
#define swc_lib_db_protocol_req_RsColumnDelete_h

#include "swcdb/lib/db/Protocol/params/ColumnId.h"

namespace SWC { namespace Protocol { namespace Req {
  

class RsColumnDelete : public ConnQueue::ReqBase  {
  public:

  RsColumnDelete(server::Mngr::RsStatusPtr rs, int64_t cid) 
                : ConnQueue::ReqBase(false), rs(rs), cid(cid) { 
      
    Params::ColumnId params = Params::ColumnId(cid);
    CommHeader header(Command::REQ_RS_COLUMN_DELETE, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~RsColumnDelete() { }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(was_called)
      return;

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == Command::REQ_RS_COLUMN_DELETE){
      int err = ev->error != Error::OK ? ev->error : response_code(ev);
      if(err == Error::OK) {
        was_called = true;
        remove(err);
      } else 
        request_again();
      return;
    }
  }

  void handle_no_conn() override { 
    remove(Error::OK);
  }
  
  void remove(int err);


  private:

  server::Mngr::RsStatusPtr   rs;
  int64_t                     cid;
};

}}}

#endif // swc_lib_db_protocol_req_RsColumnDelete_h
