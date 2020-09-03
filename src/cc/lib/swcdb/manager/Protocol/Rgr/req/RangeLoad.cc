
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


RangeLoad::RangeLoad(const Manager::Ranger::Ptr& rgr, 
                     const Manager::Range::Ptr& range,
                     const DB::Schema::Ptr& schema) 
                    : client::ConnQueue::ReqBase(false), 
                      rgr(rgr), range(range), 
                      schema_revision(schema->revision) {
  cbp = CommBuf::make(Params::RangeLoad(range->cfg->cid, range->rid, schema));
  cbp->header.set(RANGE_LOAD, 3600000);
}
  
RangeLoad::~RangeLoad() { }

void RangeLoad::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(!valid() || ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::RangeLoaded params(range->cfg->key_seq);
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      params.decode(&ptr, &remain);

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }
  loaded(err, false, params.interval);
}

bool RangeLoad::valid() {
  return !range->deleted();
}
  
void RangeLoad::handle_no_conn() {
  loaded(Error::COMM_NOT_CONNECTED, true, 
         DB::Cells::Interval(range->cfg->key_seq));
}

  
void RangeLoad::loaded(int err, bool failure, 
                       const DB::Cells::Interval& intval) {
  auto col = Env::Mngr::columns()->get_column(err, range->cfg->cid);
  if(!col)
    return Env::Mngr::rangers()->range_loaded(
      rgr, range, Error::COLUMN_MARKED_REMOVED, failure);
  if(!err)
    col->change_rgr_schema(rgr->rgrid, schema_revision);
                           
  else if(err == Error::COLUMN_SCHEMA_MISSING)
    col->remove_rgr_schema(rgr->rgrid);

  Env::Mngr::rangers()->range_loaded(rgr, range, err, failure, false);
  col->sort(range, intval);
  SWC_LOGF(LOG_INFO, "RANGE-STATUS %d(%s), %s", 
            err, Error::get_text(err), range->to_string().c_str());
}


}}}}