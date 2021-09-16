/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RgrUpdate_h
#define swcdb_manager_Protocol_handlers_RgrUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/RgrUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


struct RgrUpdate {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  RgrUpdate(const Comm::ConnHandlerPtr& a_conn,
            const Comm::Event::Ptr& a_ev) noexcept
            : conn(a_conn), ev(a_ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      Params::RgrUpdate params;
      params.decode(&ptr, &remain);

      conn->response_ok(ev);
      Env::Mngr::rangers()->update_status(params.hosts, params.sync_all);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      conn->send_error(e.code(), "", ev);
    }
  }

};


}}}}}

#endif // swcdb_manager_Protocol_handlers_RgrUpdate_h
