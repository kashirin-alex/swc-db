/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeQuerySelect_h
#define swc_app_ranger_handlers_RangeQuerySelect_h

#include "swcdb/lib/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "../callbacks/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {

class RangeQuerySelect : public AppHandler {
  public:

  RangeQuerySelect(ConnHandlerPtr conn, EventPtr ev)
                  : AppHandler(conn, ev) { }


  void run() override {

    int err = Error::OK;
    Params::RangeQuerySelectReq params;
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
      std::cout << "RangeQuerySelect, req: cid=" << params.cid << " rid=" << params.rid 
                << " " << params.interval.to_string() << "\n";

      DB::SchemaPtr schema = Env::Schemas::get()->get(params.cid);
      if(!err && schema == nullptr) { 
        // cannot be happening, range-loaded always with schema
        err = Error::COLUMN_SCHEMA_MISSING;
      }

      if(err) {
        m_conn->send_error(err, "", m_ev);
        return;
      }

      auto cb = std::make_shared<server::Rgr::Callback::RangeQuerySelect>(
        m_conn, m_ev, range);

      cb->req = DB::Cells::ReqScan::make(
        DB::Specs::Interval::make_ptr(params.interval),
        DB::Cells::Mutable::make(
          params.interval.flags.limit, 
          schema->cell_versions, 
          schema->cell_ttl, 
          schema->col_type
        ),
        [cb](int err){cb->response(err);}
      );
      
      range->scan(cb->req);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeQuerySelect_h