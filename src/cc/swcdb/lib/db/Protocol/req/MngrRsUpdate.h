
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrRsUpdate_h
#define swc_lib_db_protocol_req_MngrRsUpdate_h

#include "swcdb/lib/db/Protocol/params/MngrRsUpdate.h"

namespace SWC {
namespace Protocol {
namespace Req {

class MngrRsUpdate : public ConnQueue::ReqBase {
  public:

  MngrRsUpdate(server::Mngr::RsStatusList &hosts, bool sync_all) {
    Params::MngrRsUpdate params(hosts, sync_all);
    CommHeader header(Command::MNGR_UPDATE_RANGESERVERS, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrRsUpdate() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == Protocol::Command::MNGR_UPDATE_RANGESERVERS 
       && Protocol::response_code(ev) == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }

};

}}}

#endif // swc_lib_db_protocol_req_MngrRsUpdate_h
