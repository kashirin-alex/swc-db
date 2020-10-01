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

    int err = Error::OK;
    Env::Rgr::columns()->unload_range(err, params.cid, params.rid, 
      [conn, ev](int err) {
        if(!err)
          conn->response_ok(ev);
        else
          conn->send_error(err, "", ev);
      }
    );
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeUnload_h
