/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_Protocol_handlers_ColumnDelete_h
#define swcdb_ranger_Protocol_handlers_ColumnDelete_h

#include "swcdb/db/Protocol/Common/params/ColumnId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void column_delete(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColumnId params;
    params.decode(&ptr, &remain);

    RangerEnv::columns()
      ->remove(new Ranger::ColumnsReqDelete(params.cid, conn, ev));
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_ColumnDelete_h