/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_RangeLoad_h
#define swc_app_rangeserver_handlers_RangeLoad_h

#include "swcdb/lib/db/Protocol/params/RsRangeLoad.h"
#include "swcdb/lib/rangeserver/callbacks/RangeLoaded.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class RangeLoad : public AppHandler {
  public:

  RangeLoad(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::RsRangeLoad params;
      params.decode(&ptr, &remain);

      if(params.schema != nullptr) {
        Env::Schemas::get()->replace(params.schema);
        if(!Env::RsData::is_shuttingdown())
          HT_DEBUGF("updated %s", params.schema->to_string().c_str());
      }
      
      int err = Error::OK;
      Env::RsColumns::get()->load_range(
        err,
        params.cid, params.rid, 
        std::make_shared<Callback::RangeLoaded>(m_conn, m_ev)
      );
      
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_RangeLoad_h