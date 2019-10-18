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

  RangeLocate(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    int err = Error::OK;
    Params::RangeLocateReq params;
    server::Rgr::Range::Ptr range;

    try {
      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;
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
      
      DB::SchemaPtr schema = Env::Schemas::get()->get(params.cid);
      if(schema == nullptr) { 
        // cannot be happening, range-loaded always with schema
        err = Error::COLUMN_SCHEMA_MISSING;
      }

      if(err) {
        m_conn->send_error(err, "", m_ev);
        return;
      }
;
      auto cb = std::make_shared<server::Rgr::Callback::RangeLocateScan>(
        m_conn, m_ev, range);

      params.interval.flags.limit = 2;
      auto spec = DB::Specs::Interval::make_ptr(params.interval);
      spec->key_finish.copy(spec->key_start);
      spec->key_finish.set(-1, Condition::LE);
      if(range->type != Types::Range::DATA) 
        spec->key_finish.set(0, Condition::EQ);

      cb->req = DB::Cells::ReqScan::make(
        spec,
        DB::Cells::Mutable::make(
          params.interval.flags.limit, 
          schema->cell_versions, 
          schema->cell_ttl, 
          schema->col_type
        ),
        [cb](int err){cb->response(err);},
        [cb](const DB::Cells::Cell& cell) {
            
          size_t remain = cell.vlen;
          const uint8_t * ptr = cell.value;
          Serialization::decode_vi64(&ptr, &remain);
          DB::Cell::Key key_end;
          key_end.decode(&ptr, &remain);
          /*
          std::cout << "cell begin: "<< cell.key.to_string() << "\n";
          std::cout << "spec begin: " << cb->req->spec->key_start.to_string() << "\n";
          std::cout << "cell end:   "<< key_end.to_string() << "\n";
          std::cout << "spec end:   " << cb->req->spec->key_finish.to_string() << "\n";
          */
          if(!cb->req->spec->key_start.is_matching(cell.key))
            return false;
          if(cb->req->cells->size() == 1)
            return true; // next_key
          return key_end.empty() || 
                 cb->req->spec->key_finish.is_matching(key_end);
        }
      );
      
      //std::cout << " handler::RangeLocate,  " << cb->req->to_string() << "\n";
      
      range->scan(cb->req);
  
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeLocate_h