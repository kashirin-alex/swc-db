
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/db/Protocol/Common/params/ColumnsInterval.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnsUnload.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {
  

ColumnsUnload::ColumnsUnload(const Manager::Ranger::Ptr& rgr, 
                             cid_t cid_begin, cid_t cid_end)
                            : client::ConnQueue::ReqBase(false), 
                              rgr(rgr), 
                              cid_begin(cid_begin), cid_end(cid_end) {
  cbp = Buffers::make(Common::Params::ColumnsInterval(cid_begin, cid_end));
  cbp->header.set(COLUMNS_UNLOAD, 60000);
}
  
ColumnsUnload::~ColumnsUnload() { }

void ColumnsUnload::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnsUnloadRsp rsp_params;
  int err = ev->error;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);
      err = rsp_params.err;

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }
  
  if(err)
    return handle_no_conn();

  if(rsp_params.columns.empty())
    return;

  rgrid_t rgrid;
  Manager::Ranger::Ptr assigned;
  for(auto& c : rsp_params.columns) {
    auto col = Env::Mngr::columns()->get_column(err = Error::OK, c.first);
    if(err || !col)
      continue;

    for(auto& r : c.second) {

      auto range = col->get_range(err, r);
      if(!range || !range->assigned() || !(rgrid = range->get_rgr_id()))
        continue;

      assigned = Env::Mngr::rangers()->rgr_get(rgrid);
      if(!assigned || assigned->state != Manager::Ranger::State::ACK)
        continue;

      assigned->put(std::make_shared<RangeUnload>(assigned, col, range));
    }
  }
}

void ColumnsUnload::handle_no_conn() {
  ++rgr->failures;
}
  

}}}}}
