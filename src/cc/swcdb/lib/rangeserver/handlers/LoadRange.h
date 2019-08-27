/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_LoadRange_h
#define swc_app_rangeserver_handlers_LoadRange_h

#include "swcdb/lib/db/Protocol/params/ColRangeId.h"
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

      Protocol::Params::ColRangeId params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      Env::RsColumns::get()->load_range(
        params.cid, params.rid, 
         std::make_shared<Callback::RangeLoaded>(m_conn, m_ev));

      // params.create?
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_rangeserver_handlers_LoadRange_h