/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeLoad_h
#define swc_app_ranger_handlers_RangeLoad_h

#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"
#include "swcdb/ranger/callbacks/RangeLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void range_load(ConnHandlerPtr conn, Event::Ptr ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeLoad params;
    params.decode(&ptr, &remain);

    if(params.schema != nullptr) {
      Env::Schemas::get()->replace(params.schema);
      if(!Env::RgrData::is_shuttingdown())
        SWC_LOGF(LOG_DEBUG, "updated %s", params.schema->to_string().c_str());
    }
      
    int err = Error::OK;
    Env::RgrColumns::get()->load_range(
      err,
      params.cid, params.rid, 
      std::make_shared<server::Rgr::Callback::RangeLoaded>(
        conn, ev, params.cid, params.rid)
    );
       
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_app_ranger_handlers_RangeLoad_h