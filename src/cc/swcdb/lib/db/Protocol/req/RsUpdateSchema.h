
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsUpdateSchema_h
#define swc_lib_db_protocol_req_RsUpdateSchema_h

#include "swcdb/lib/db/Protocol/params/RsUpdateSchema.h"

namespace SWC { namespace Protocol { namespace Req {


class RsUpdateSchema : public ConnQueue::ReqBase {
  public:

  RsUpdateSchema(server::Mngr::RsStatusPtr rs, DB::SchemaPtr schema) 
              : ConnQueue::ReqBase(false), rs(rs), schema(schema) {

    Params::RsUpdateSchema params = Params::RsUpdateSchema(schema);
    CommHeader header(Command::REQ_RS_SCHEMA_UPDATE, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~RsUpdateSchema() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == Command::REQ_RS_SCHEMA_UPDATE){
      int err = ev->error != Error::OK? ev->error: response_code(ev);
      if(err == Error::OK)
        Env::MngrColumns::get()->get_column(err, schema->cid, false)
                           ->add_rs(rs->rs_id, schema->revision);
      return; 
    }
  }

  private:

  server::Mngr::RsStatusPtr rs;
  DB::SchemaPtr schema; 
   
};

}}}

#endif // swc_lib_db_protocol_req_RsUpdateSchema_h
