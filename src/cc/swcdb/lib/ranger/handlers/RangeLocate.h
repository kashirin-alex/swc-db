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
      if(err) {
        m_conn->send_error(err, "", m_ev);
        return;
      }

      params.interval->flags.limit = 2;

      auto ptr = std::make_shared<server::Rgr::Callback::RangeLocateScan>(
        m_conn, m_ev, params.cid);

      ptr->req =  DB::Cells::ReqScan::make(
        DB::Specs::Interval::make_ptr(params.interval),
        DB::Cells::Mutable::make(2, 1, 0, SWC::Types::Column::PLAIN),
        [ptr](int err){ptr->response(err);}
      );

      range->scan(ptr->req);
  
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeLocate_h