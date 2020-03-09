
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 


#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"
#include "swcdb/db/Protocol/Common/params/ColumnId.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

ColumnDelete::ColumnDelete(Manager::Ranger::Ptr rgr, int64_t cid) 
                          : client::ConnQueue::ReqBase(false), 
                            rgr(rgr), cid(cid) {
  cbp = CommBuf::make(Common::Params::ColumnId(cid));
  cbp->header.set(COLUMN_DELETE, 60000);
}
  
ColumnDelete::~ColumnDelete() { }
  
void ColumnDelete::handle(ConnHandlerPtr conn, Event::Ptr& ev) {

  if(was_called)
    return;

  if(ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  if(ev->header.command == COLUMN_DELETE) {
    int err = ev->error != Error::OK ? ev->error : ev->response_code();
    if(err == Error::OK) {
      was_called = true;
      remove(err);
    } else 
      request_again();
    return;
  }
}

void ColumnDelete::handle_no_conn() { 
  remove(Error::OK);
}
  
void ColumnDelete::remove(int err) {
  Env::Mngr::mngd_columns()->remove(err, cid, rgr->id);  
}

}}}}
