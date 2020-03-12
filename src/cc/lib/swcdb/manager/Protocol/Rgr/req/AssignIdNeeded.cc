
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#include "swcdb/manager/Protocol/Rgr/req/AssignIdNeeded.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

AssignIdNeeded::AssignIdNeeded(Manager::Ranger::Ptr rs_chk, 
                               Manager::Ranger::Ptr rs_nxt, 
                               Manager::Range::Ptr range) 
                              : client::ConnQueue::ReqBase(false), 
                                rs_chk(rs_chk), rs_nxt(rs_nxt), range(range) {
  cbp = CommBuf::make();
  cbp->header.set(ASSIGN_ID_NEEDED, 60000);
}
  
AssignIdNeeded::~AssignIdNeeded() { }

void AssignIdNeeded::handle(ConnHandlerPtr conn, Event::Ptr& ev) {

  if(was_called)
    return;
  was_called = true;

  if(!valid() || ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
      return;
  }

  if(ev->header.command == ASSIGN_ID_NEEDED) {
    rsp(ev->error != Error::OK ? ev->error : ev->response_code());
    return;
  }
}

bool AssignIdNeeded::valid() {
  return !range->deleted();
}

void AssignIdNeeded::handle_no_conn() {
  rsp(Error::COMM_NOT_CONNECTED);
};

void AssignIdNeeded::rsp(int err) {
  if(!err) 
    // RsId assignment on the way, put range back as not assigned 
    Env::Mngr::rangers()->range_loaded(
      rs_nxt, range, Error::RS_NOT_READY);
  else
    Env::Mngr::rangers()->assign_range(rs_nxt, range);

    // the same cond to reqs pending_id
  Env::Mngr::rangers()->assign_range_chk_last(err, rs_chk);
}

}}}}
