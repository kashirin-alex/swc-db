/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnCompact_h
#define swcdb_broker_Protocol_handlers_ColumnCompact_h

#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


void column_compact(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Protocol::Mngr::Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    Protocol::Mngr::Req::ColumnCompact::request(
      Env::Clients::get(),
      params,
      [conn, ev]
      (const client::ConnQueue::ReqBase::Ptr&,
       const Protocol::Mngr::Params::ColumnCompactRsp& rsp) noexcept {
        conn->send_response(Buffers::make(ev, rsp));
        Env::Bkr::in_process(-1);
      }
    );

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_response(
      Buffers::make(ev, Protocol::Mngr::Params::ColumnCompactRsp(e.code())));
    Env::Bkr::in_process(-1);
  }

}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnCompact_h