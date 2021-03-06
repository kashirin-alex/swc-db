/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnMng_h
#define swcdb_manager_Protocol_handlers_ColumnMng_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void column_mng(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Manager::MngdColumns::ColumnReq::Ptr req(
      new Manager::MngdColumns::ColumnReq(conn, ev));
    req->decode(&ptr, &remain);

    if(Env::Mngr::mngd_columns()->is_schemas_mngr(err) && !err)
      return Env::Mngr::mngd_columns()->action(req);
    if(!err)
      err = Error::MNGR_NOT_ACTIVE;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  conn->send_error(err , "", ev);
}


}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnMng_h
