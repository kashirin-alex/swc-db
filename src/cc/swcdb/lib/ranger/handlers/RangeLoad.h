/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeLoad_h
#define swc_app_ranger_handlers_RangeLoad_h

#include "swcdb/lib/db/Protocol/Rgr/params/RangeLoad.h"
#include "../callbacks/RangeLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


class RangeLoad : public AppHandler {
  public:

  RangeLoad(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Params::RangeLoad params;
      params.decode(&ptr, &remain);

      if(params.schema != nullptr) {
        Env::Schemas::get()->replace(params.schema);
        if(!Env::RgrData::is_shuttingdown())
          HT_DEBUGF("updated %s", params.schema->to_string().c_str());
      }
      
      int err = Error::OK;
      Env::RgrColumns::get()->load_range(
        err,
        params.cid, params.rid, 
        std::make_shared<server::Rgr::Callback::RangeLoaded>(
          m_conn, m_ev, params.cid, params.rid)
      );
       
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeLoad_h