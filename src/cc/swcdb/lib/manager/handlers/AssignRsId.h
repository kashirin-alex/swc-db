/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_AssignRsId_h
#define swc_app_manager_handlers_AssignRsId_h

#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/manager/MngrRangeServers.h"

#include "swcdb/lib/db/Protocol/Commands.h"

#include <memory>

namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class AssignRsId : public AppHandler {
  public:

    AssignRsId(ConnHandlerPtr conn, EventPtr ev,
               MngrRangeServersPtr mngr_rs
               ): AppHandler(conn, ev), m_mngr_rs(mngr_rs) {}


  void run() {
    /* 
    Response::Callback::CreateScanner cb(m_comm, m_event);

    try {
    const uint8_t *ptr = m_event->payload;
    size_t remain = m_event->payload_len;
    QueryCache::Key key;
    Lib::RangeServer::Request::Parameters::CreateScanner params;

    const uint8_t *base = ptr;
    params.decode(&ptr, &remain);

    if (params.scan_spec().cacheable()) {
      md5_csum((unsigned char *)base, ptr-base,
               reinterpret_cast<unsigned char *>(key.digest));
      m_range_server->create_scanner(&cb, params.table(), params.range_spec(),
                                     params.scan_spec(), &key);
    }
    else
      m_range_server->create_scanner(&cb, params.table(), params.range_spec(),
                                     params.scan_spec(), 0);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      m_ev_ctx.error(e.code(), e.what());
    }
    */
  }

  private:
    MngrRangeServersPtr m_mngr_rs;
  };
  

}}}}

#endif // swc_app_manager_handlers_AssignRsId_h