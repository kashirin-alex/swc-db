/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeUnload_h
#define swcdb_ranger_Protocol_handlers_RangeUnload_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_unload(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Common::Params::ColRangeId params;
    params.decode(&ptr, &remain);

    auto col = Env::Rgr::columns()->get_column(params.cid);
    if(col) {
      col->add_managing(
        std::make_shared<Ranger::Callback::RangeUnload>(
          conn, ev, params.cid, params.rid, true ));

    } else {
      conn->response_ok(ev);
    }
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    conn->send_error(e.code(), "", ev);
  }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeUnload_h
