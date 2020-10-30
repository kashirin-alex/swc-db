/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnList_h
#define swcdb_manager_Protocol_handlers_ColumnList_h

#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void column_list(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  Params::ColumnListRsp rsp;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnListReq req_params;
    req_params.decode(&ptr, &remain); // opt for list cid range

    if(Env::Mngr::mngd_columns()->is_schemas_mngr(err) && !err)
      req_params.patterns.empty()
        ? Env::Mngr::schemas()->all(rsp.schemas)
        : Env::Mngr::schemas()->matching(req_params.patterns, rsp.schemas);
    else if(!err)
      err = Error::MNGR_NOT_ACTIVE;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rsp, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}



}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnList_h
