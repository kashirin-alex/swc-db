
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrUpdateColumn_h
#define swc_lib_db_protocol_req_MngrUpdateColumn_h

#include "swcdb/lib/db/Protocol/params/MngrUpdateColumn.h"

namespace SWC {
namespace Protocol {
namespace Req {

class MngrUpdateColumn : public ConnQueue::ReqBase {
  public:

  MngrUpdateColumn(Params::MngColumn::Function function, 
                  int64_t cid, int err) {

    Params::MngrUpdateColumn params(function, cid, err);
    CommHeader header(Command::MNGR_UPDATE_COLUMN, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrUpdateColumn() { }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == Protocol::Command::MNGR_UPDATE_COLUMN 
       && Protocol::response_code(ev) == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }
  
};

}}}

#endif // swc_lib_db_protocol_req_MngrUpdateColumn_h
