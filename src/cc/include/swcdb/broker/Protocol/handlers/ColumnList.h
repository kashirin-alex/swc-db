/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnList_h
#define swcdb_broker_Protocol_handlers_ColumnList_h

#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


void column_list(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Protocol::Mngr::Params::ColumnListRsp rsp;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Protocol::Mngr::Params::ColumnListReq req_params;
    req_params.decode(&ptr, &remain);
    Env::Clients::get()->get_schema(err, req_params.patterns, rsp.schemas);
    if(err == Error::COLUMN_SCHEMA_MISSING)
      err = Error::OK;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rsp, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

  Env::Bkr::processed();
}



}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnList_h
