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
  std::vector<DB::Schema::Ptr> schemas;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnListReq req_params;
    req_params.decode(&ptr, &remain); // opt for list cid range

    if(Env::Mngr::mngd_columns()->is_schemas_mngr(err) && !err)
      req_params.patterns.names.empty() &&
      req_params.patterns.tags.comp == Condition::NONE
        ? Env::Mngr::schemas()->all(schemas)
        : Env::Mngr::schemas()->matching(req_params.patterns, schemas);
    else if(!err)
      err = Error::MNGR_NOT_ACTIVE;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  if(err) {
    auto cbp = Buffers::make(ev, 4);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } else if(schemas.size() <= 1000) {
    Params::ColumnListRsp rsp;
    rsp.schemas = std::move(schemas);
    auto cbp = Buffers::make(ev, rsp, 4);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } else {
    size_t i = 0;
    bool more;
    do {
      Params::ColumnListRsp rsp;
      rsp.expected = schemas.size();
      auto it = schemas.cbegin() + i;
      more = (i += 1000) < rsp.expected;
      rsp.schemas.assign(it, (more ? (it + 1000) : schemas.cend()));
      auto cbp = Buffers::make(ev, rsp, 4);
      if(more)
        cbp->header.flags |= Comm::Header::FLAG_RESPONSE_PARTIAL_BIT;
      cbp->append_i32(err);
      conn->send_response(cbp);
    } while(more);
  }

}



}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnList_h
