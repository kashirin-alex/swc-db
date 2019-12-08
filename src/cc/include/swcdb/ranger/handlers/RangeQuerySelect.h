/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeQuerySelect_h
#define swc_app_ranger_handlers_RangeQuerySelect_h

#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "swcdb/ranger/callbacks/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_query_select(ConnHandlerPtr conn, Event::Ptr ev) {
  int err = Error::OK;
  Params::RangeQuerySelectReq params;
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

    range->scan(
      std::make_shared<server::Rgr::Callback::RangeQuerySelect>(
        conn, ev,

        DB::Specs::Interval::make_ptr(params.interval),
        DB::Cells::Mutable::make(
          params.interval.flags.limit, 
          params.interval.flags.max_versions != 0 ? 
          params.interval.flags.max_versions : schema->cell_versions, 
          schema->cell_ttl, 
          schema->col_type
        ), 
        
        range,
        params.limit_buffer_sz
      )
    );
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_app_ranger_handlers_RangeQuerySelect_h