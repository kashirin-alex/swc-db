/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_RangeLocate_h
#define swc_app_rangeserver_handlers_RangeLocate_h

#include "swcdb/lib/db/Protocol/params/RsRangeLocate.h"
#include "swcdb/lib/rangeserver/callbacks/RangeLocateScan.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class RangeLocate : public AppHandler {
  public:

  RangeLocate(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    int err = Error::OK;
    Protocol::Params::RsRangeLocateReq params;
    RangePtr range;

    try {
      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;
      params.decode(&ptr, &remain);

      range =  Env::RsColumns::get()->get_range(
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
      range->scan(
        DB::Specs::Interval::make_ptr(params.interval), 
        std::make_shared<Callback::RangeLocateScan>(
          m_conn, m_ev, params.cid)
      );
  
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_RangeLocate_h