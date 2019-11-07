
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_ColumnUpdate_h
#define swc_lib_db_protocol_rgr_req_ColumnUpdate_h

#include "../params/ColumnUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

class ColumnUpdate : public Common::Req::ConnQueue::ReqBase {
  public:

  ColumnUpdate(server::Mngr::Ranger::Ptr rgr, DB::Schema::Ptr schema) 
              : Common::Req::ConnQueue::ReqBase(false), 
                rgr(rgr), schema(schema) {
    cbp = CommBuf::make(Params::ColumnUpdate(schema));
    cbp->header.set(SCHEMA_UPDATE, 60000);
  }
  
  virtual ~ColumnUpdate() { }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == SCHEMA_UPDATE){
      updated(ev->error != Error::OK? ev->error: response_code(ev), false);
      return; 
    }
  }
  
  void handle_no_conn() override {
    updated(Error::COMM_NOT_CONNECTED, true);
  }

  void updated(int err, bool failure);

  private:

  server::Mngr::Ranger::Ptr   rgr;
  DB::Schema::Ptr             schema; 
   
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_ColumnUpdate_h
