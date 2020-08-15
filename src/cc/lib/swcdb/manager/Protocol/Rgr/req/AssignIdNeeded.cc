
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/manager/Protocol/Rgr/req/AssignIdNeeded.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

AssignIdNeeded::AssignIdNeeded(const Manager::Ranger::Ptr& rs_chk, 
                               const Manager::Ranger::Ptr& rs_nxt, 
                               const Manager::Range::Ptr& range) 
                              : client::ConnQueue::ReqBase(false), 
                                rs_nxt(rs_nxt), range(range), rs_chk(rs_chk) {
  cbp = CommBuf::make();
  cbp->header.set(ASSIGN_ID_NEEDED, 60000);
}
  
AssignIdNeeded::~AssignIdNeeded() { }

void AssignIdNeeded::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(!valid() || ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  rsp(ev->response_code());
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
