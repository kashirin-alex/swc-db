
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

ColumnUpdate::ColumnUpdate(Manager::Ranger::Ptr rgr, DB::Schema::Ptr schema)
                          : client::ConnQueue::ReqBase(false), 
                            rgr(rgr), schema(schema) {
  cbp = CommBuf::make(Params::ColumnUpdate(schema));
  cbp->header.set(SCHEMA_UPDATE, 60000);
}
  
ColumnUpdate::~ColumnUpdate() { }

void ColumnUpdate::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
      
  if(was_called)
    return;
  was_called = true;

  if(ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  if(ev->header.command == SCHEMA_UPDATE){
    updated(ev->error != Error::OK? ev->error: ev->response_code(), false);
    return; 
  }
}
  
void ColumnUpdate::handle_no_conn() {
  updated(Error::COMM_NOT_CONNECTED, true);
}

void ColumnUpdate::updated(int err, bool failure) {
  if(!err) {
    Env::Mngr::columns()->get_column(err, schema->cid, false)
                             ->change_rgr_schema(rgr->id, schema->revision);
    if(!Env::Mngr::rangers()->update(schema, false)) {
      Env::Mngr::mngd_columns()->update(
        Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY,
        schema,
        err
      );
    }
  } else if(failure) {
    ++rgr->failures;
    request_again();
  } 
}


}}}}
