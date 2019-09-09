/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_LoadRange_h
#define swc_app_rangeserver_handlers_LoadRange_h

#include "swcdb/lib/db/Protocol/params/RsLoadRange.h"
#include "swcdb/lib/rangeserver/callbacks/RangeLoaded.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class LoadRange : public AppHandler {
  public:

  LoadRange(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::RsLoadRange params;
      params.decode(&ptr, &remain);

      if(params.schema != nullptr){
        Env::Schemas::get()->replace(params.schema);
      }

      int err = Error::OK;
      Env::RsColumns::get()->load_range(
        err,
        params.cid, params.rid, 
        std::make_shared<Callback::RangeLoaded>(m_conn, m_ev)
      );

      // params.create?
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_LoadRange_h