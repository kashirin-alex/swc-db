/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_ColumnUpdate_h
#define swcdb_ranger_Protocol_handlers_ColumnUpdate_h

#include "swcdb/db/Protocol/Rgr/params/ColumnUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void column_update(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnUpdate params;
    params.decode(&ptr, &remain);
    
    if(!Env::Rgr::is_not_accepting()) {
      auto col = Env::Rgr::columns()->get_column(params.schema->cid);
      if(col) {
        col->schema_update(*params.schema.get());
        SWC_LOG_OUT(LOG_DEBUG,
          col->cfg->print(SWC_LOG_OSTREAM << "updated "); );
      }
    }
    conn->response_ok(ev);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code(), "", ev);
  }
  
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_ColumnUpdate_h
