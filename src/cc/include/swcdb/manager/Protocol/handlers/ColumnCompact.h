/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnCompact_h
#define swcdb_manager_Protocol_handlers_ColumnCompact_h

#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void column_compact(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::ColumnCompactRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    auto col = Env::Mngr::mngd_columns()->get_column(
      rsp_params.err, params.cid);
    if(!rsp_params.err)
      Env::Mngr::rangers()->column_compact(col);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  
  conn->send_response(Buffers::make(ev, rsp_params));
}


}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnCompact_h