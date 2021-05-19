/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnMng_h
#define swcdb_broker_Protocol_handlers_ColumnMng_h

#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


void column_mng(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Protocol::Mngr::Params::ColumnMng params;
    params.decode(&ptr, &remain);

    Protocol::Mngr::Req::ColumnMng::request(
      Env::Clients::get(), params,
      [conn, ev] (const client::ConnQueue::ReqBase::Ptr&, int err) noexcept {
        err ? conn->send_error(err , "", ev) : conn->response_ok(ev);
        Env::Bkr::in_process(-1);
      }
    );

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code() , "", ev);
    Env::Bkr::in_process(-1);
  }
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnMng_h
