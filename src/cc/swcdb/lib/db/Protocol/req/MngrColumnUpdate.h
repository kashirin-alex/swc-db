
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrColumnUpdate_h
#define swc_lib_db_protocol_req_MngrColumnUpdate_h

#include "swcdb/lib/db/Protocol/params/MngrColumnUpdate.h"

namespace SWC {
namespace Protocol {
namespace Req {

class MngrColumnUpdate : public ConnQueue::ReqBase {
  public:

  MngrColumnUpdate(Params::ColumnMng::Function function, 
                   DB::SchemaPtr schema, int err) {

    Params::MngrColumnUpdate params(function, schema, err);
    CommHeader header(Mngr::COLUMN_UPDATE, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrColumnUpdate() { }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == Mngr::COLUMN_UPDATE 
       && response_code(ev) == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }
  
};

}}}

#endif // swc_lib_db_protocol_req_MngrColumnUpdate_h
