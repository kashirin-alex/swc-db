/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_ColumnDelete_h
#define swcdb_ranger_Protocol_handlers_ColumnDelete_h

#include "swcdb/db/Protocol/Common/params/ColumnId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void column_delete(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColumnId params;
    params.decode(&ptr, &remain);

    RangerEnv::columns()
      ->remove(new Ranger::ColumnsReqDelete(params.cid, conn, ev));

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_ColumnDelete_h