/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeLocate_h
#define swc_app_ranger_handlers_RangeLocate_h

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/ranger/callbacks/RangeLocateScan.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_locate(ConnHandlerPtr conn, Event::Ptr ev) {
  int err = Error::OK;
  Params::RangeLocateReq params;
  server::Rgr::Range::Ptr range;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    range =  Env::RgrColumns::get()->get_range(
      err, params.cid, params.rid, false);
    
    if(!err && (range == nullptr || !range->is_loaded()))
      err = Error::RS_NOT_LOADED_RANGE;

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  try{

    DB::Schema::Ptr schema = Env::Schemas::get()->get(params.cid);
    if(!err && schema == nullptr) { 
      // cannot be happening, range-loaded always with schema
      err = Error::COLUMN_SCHEMA_MISSING;
    }

    if(err) {
      conn->send_error(err, "", ev);
      return;
    }

    auto spec = DB::Specs::Interval::make_ptr(params.interval);
    spec->key_finish.copy(spec->key_start);
    spec->key_finish.set(-1, Condition::LE);
    if(range->type != Types::Range::DATA) 
      spec->key_finish.set(0, Condition::EQ);
    spec->flags.limit = 2;

    range->scan(
      std::make_shared<server::Rgr::Callback::RangeLocateScan>(
        conn, ev, 
        spec,  
        DB::Cells::Mutable::make(
          params.interval.flags.limit, 
          schema->cell_versions, 
          schema->cell_ttl, 
          schema->col_type
        ), 
        range
      )
    );
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_app_ranger_handlers_RangeLocate_h