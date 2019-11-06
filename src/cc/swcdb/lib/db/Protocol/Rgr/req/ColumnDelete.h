
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_ColumnDelete_h
#define swc_lib_db_protocol_rgr_req_ColumnDelete_h

#include "swcdb/lib/db/Protocol/Common/params/ColumnId.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnDelete : public Common::Req::ConnQueue::ReqBase  {
  public:

  ColumnDelete(server::Mngr::RangerPtr rgr, int64_t cid) 
              : Common::Req::ConnQueue::ReqBase(false), 
                rgr(rgr), cid(cid) {
    cbp = CommBuf::make(Common::Params::ColumnId(cid));
    cbp->header.set(COLUMN_DELETE, 60000);
  }
  
  virtual ~ColumnDelete() { }
  
  void handle(ConnHandlerPtr conn, Event::Ptr &ev) override {

    if(was_called)
      return;

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == COLUMN_DELETE) {
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

  server::Mngr::RangerPtr  rgr;
  int64_t                  cid;
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_ColumnDelete_h
