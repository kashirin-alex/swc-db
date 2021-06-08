/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeLoad_h
#define swcdb_ranger_Protocol_handlers_RangeLoad_h

#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"
#include "swcdb/ranger/callbacks/RangeLoad.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_load(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeLoad params;
    params.decode(&ptr, &remain);

    Env::Rgr::columns()->load_range(
      *params.schema.get(),
      Ranger::Callback::RangeLoad::Ptr(
        new Ranger::Callback::RangeLoad(conn, ev, params.cid, params.rid))
    );

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code(), "", ev);
  }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeLoad_h
