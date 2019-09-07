
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsLoadRange_h
#define swc_lib_db_protocol_req_RsLoadRange_h

#include "swcdb/lib/db/Protocol/params/ColRangeId.h"

namespace SWC { namespace Protocol { namespace Req {


class RsLoadRange : public ConnQueue::ReqBase {
  public:

  RsLoadRange(server::Mngr::RsStatusPtr rs, server::Mngr::RangePtr range) 
              : ConnQueue::ReqBase(false), rs(rs), range(range) {
    Params::ColRangeId params = Params::ColRangeId(range->cid, range->rid);
    CommHeader header(Command::REQ_RS_LOAD_RANGE, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~RsLoadRange() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == Command::REQ_RS_LOAD_RANGE){
      loaded(ev->error != Error::OK? ev->error: response_code(ev), false); 
      return; 
    }
  }

  bool valid() override {
    return !range->deleted();
  }
  
  void handle_no_conn() override {
    loaded(Error::COMM_NOT_CONNECTED, true);
  }

  void loaded(int err, bool failure);


  private:

  server::Mngr::RsStatusPtr rs;
  server::Mngr::RangePtr    range;
   
};

}}}

#endif // swc_lib_db_protocol_req_RsLoadRange_h
