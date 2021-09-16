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


struct RangeLoad {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  RangeLoad(const Comm::ConnHandlerPtr& a_conn,
            const Comm::Event::Ptr& a_ev) noexcept
            : conn(a_conn), ev(a_ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      Params::RangeLoad params;
      params.decode(&ptr, &remain);

      Env::Rgr::columns()->load_range(
        params.schema_primitives,
        Ranger::Callback::RangeLoad::Ptr(
          new Ranger::Callback::RangeLoad(
            conn, ev, params.schema_primitives.cid, params.rid))
      );

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      conn->send_error(e.code(), "", ev);
    }
  }

};


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeLoad_h
