
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


RangeLoad::RangeLoad(Manager::Ranger::Ptr rgr, Manager::Range::Ptr range,
                     DB::Schema::Ptr schema) 
                    : client::ConnQueue::ReqBase(false), 
                      rgr(rgr), range(range), 
                      schema_revision(schema->revision) {
  cbp = CommBuf::make(Params::RangeLoad(range->cfg->cid, range->rid, schema));
  cbp->header.set(RANGE_LOAD, 3600000);
}
  
RangeLoad::~RangeLoad() { }

void RangeLoad::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
      
  if(was_called)
    return;
  was_called = true;

  if(!valid() || ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  if(ev->header.command == RANGE_LOAD) {
    int err = ev->error != Error::OK? ev->error: ev->response_code();
    if(err)
      return loaded(err, false, DB::Cells::Interval(range->cfg->key_seq));
      
    const uint8_t *ptr = ev->data.base+4;
    size_t remain = ev->data.size-4;
    Params::RangeLoaded params(range->cfg->key_seq);
    params.decode(&ptr, &remain);
    loaded(err, false, params.interval); 
  }
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