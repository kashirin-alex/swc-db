
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"
#include "swcdb/db/Protocol/Common/params/ColumnId.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {
  

ColumnDelete::ColumnDelete(const Manager::Ranger::Ptr& rgr, cid_t cid,
                           uint64_t req_id)
          : client::ConnQueue::ReqBase(
              false,
              Buffers::make(
                Common::Params::ColumnId(cid), 0, COLUMN_DELETE, 60000)
            ), 
            rgr(rgr), cid(cid), req_id(req_id) {
}
  
ColumnDelete::~ColumnDelete() { }
  
void ColumnDelete::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  int err = ev->response_code();
  if(!err) {
    remove(err);
  } else {
    request_again();
  }
}

void ColumnDelete::handle_no_conn() {
  remove(Error::OK);
}
  
void ColumnDelete::remove(int err) {
  Env::Mngr::mngd_columns()->remove(err, cid, rgr->rgrid, req_id);  
}

}}}}}
