
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_mngr_req_ColumnUpdate_h
#define swc_lib_db_protocol_mngr_req_ColumnUpdate_h

#include "../params/ColumnUpdate.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

class ColumnUpdate : public Common::Req::ConnQueue::ReqBase {
  public:

  ColumnUpdate(Params::ColumnMng::Function function, DB::Schema::Ptr schema, 
               int err) {
    cbp = CommBuf::make(Params::ColumnUpdate(function, schema, err));
    cbp->header.set(COLUMN_UPDATE, 60000);
  }
  
  virtual ~ColumnUpdate() { }
  
  void handle(ConnHandlerPtr conn, Event::Ptr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == COLUMN_UPDATE && response_code(ev) == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }
  
};

}}}}

#endif // swc_lib_db_protocol_req_Update_h
