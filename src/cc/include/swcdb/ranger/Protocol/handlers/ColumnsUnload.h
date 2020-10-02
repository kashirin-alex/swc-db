/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_ColumnsUnload_h
#define swcdb_ranger_Protocol_handlers_ColumnsUnload_h

#include "swcdb/db/Protocol/Common/params/ColumnsInterval.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void columns_unload(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColumnsInterval params;
    params.decode(&ptr, &remain);
  
    Env::Rgr::columns()->unload(
      params.cid_begin, params.cid_end, 
      std::make_shared<Ranger::Callback::ColumnsUnloaded>(conn, ev)
    );
    return;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  try {
    conn->send_error(err, "", ev);
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_ColumnsUnload_h
