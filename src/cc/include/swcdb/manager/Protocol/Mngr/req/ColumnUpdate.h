
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swc_manager_Protocol_mngr_req_ColumnUpdate_h
#define swc_manager_Protocol_mngr_req_ColumnUpdate_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/manager/Protocol/Mngr/params/ColumnUpdate.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

class ColumnUpdate : public client::ConnQueue::ReqBase {
  public:

  ColumnUpdate(Params::ColumnMng::Function function, DB::Schema::Ptr schema, 
               int err) {
    cbp = CommBuf::make(Params::ColumnUpdate(function, schema, err));
    cbp->header.set(COLUMN_UPDATE, 60000);
  }
  
  virtual ~ColumnUpdate() { }
  
  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == COLUMN_UPDATE && ev->response_code() == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }
  
};

}}}}

#endif // swc_manager_Protocol_mngr_req_ColumnUpdate_h
