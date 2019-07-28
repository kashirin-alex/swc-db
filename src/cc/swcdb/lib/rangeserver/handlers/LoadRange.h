/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_LoadRange_h
#define swc_app_rangeserver_handlers_LoadRange_h

#include "swcdb/lib/db/Protocol/params/LoadRange.h"
#include "swcdb/lib/rangeserver/callbacks/RangeLoaded.h"


namespace SWC { namespace server { namespace RS {

namespace Handler {


class LoadRange : public AppHandler {
  public:

  LoadRange(ConnHandlerPtr conn, EventPtr ev, ColumnsPtr columns)
              : AppHandler(conn, ev), m_columns(columns) { }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::LoadRange params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      m_columns->load_range(
        params.cid, params.rid, 
         std::make_shared<Callback::RangeLoaded>(m_conn, m_ev));
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

  private:
  ColumnsPtr  m_columns;
};
  

}}}}

#endif // swc_app_rangeserver_handlers_LoadRange_h