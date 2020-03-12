/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_ColumnList_h
#define swc_manager_Protocol_handlers_ColumnList_h

#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {


void column_list(ConnHandlerPtr conn, Event::Ptr ev) {

  int err = Error::OK;
  Params::ColumnListRsp rsp;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnListReq req_params;
    req_params.decode(&ptr, &remain); // opt for list cid range

    Env::Mngr::mngd_columns()->is_active(err, 1, true);
    if(!err)
      Env::Mngr::schemas()->all(rsp.schemas);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  try {
    auto cbp = err ? CommBuf::make(4) : CommBuf::make(rsp, 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);
  }
  catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
}



}}}}

#endif // swc_manager_Protocol_handlers_ColumnList_h