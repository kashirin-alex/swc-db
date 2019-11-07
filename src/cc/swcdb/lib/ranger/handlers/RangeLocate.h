/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeLocate_h
#define swc_app_ranger_handlers_RangeLocate_h

#include "swcdb/lib/db/Protocol/Rgr/params/RangeLocate.h"
#include "../callbacks/RangeLocateScan.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {

class RangeLocate : public AppHandler {
  public:

  RangeLocate(ConnHandlerPtr conn, Event::Ptr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    int err = Error::OK;
    Params::RangeLocateReq params;
    server::Rgr::Range::Ptr range;

    try {
      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;
      params.decode(&ptr, &remain);

      range =  Env::RgrColumns::get()->get_range(
        err, params.cid, params.rid, false);
      
      if(range == nullptr || !range->is_loaded()){
        if(err == Error::OK)
          err = Error::RS_NOT_LOADED_RANGE;
      }
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }

    try{
      std::cout << "RangeLocate, req: cid=" << params.cid << " rid=" << params.rid 
                << " " << params.interval.to_string() << "\n";

      DB::Schema::Ptr schema = Env::Schemas::get()->get(params.cid);
      if(!err && schema == nullptr) { 
        // cannot be happening, range-loaded always with schema
        err = Error::COLUMN_SCHEMA_MISSING;
      }

      if(err) {
        m_conn->send_error(err, "", m_ev);
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
          m_conn, m_ev, 
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
  
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeLocate_h