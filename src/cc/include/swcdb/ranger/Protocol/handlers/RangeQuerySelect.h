/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeQuerySelect_h
#define swcdb_ranger_Protocol_handlers_RangeQuerySelect_h

#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "swcdb/ranger/callbacks/RangeQuerySelect.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_query_select(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Params::RangeQuerySelectReq params;
  Ranger::RangePtr range;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);

    if(!err) {
      if(!range || !range->is_loaded()) {
        err = Error::RGR_NOT_LOADED_RANGE;

      } else if(range->cfg->range_type == DB::Types::Range::DATA &&
                Env::Rgr::res().is_low_mem_state() &&
                Env::Rgr::scan_reserved_bytes()) {
        err = Error::SERVER_MEMORY_LOW;
      }
    }

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR,
      SWC_LOG_OSTREAM << e;
      ev->print(SWC_LOG_OSTREAM << "\n\t");
    );
    err = e.code();
  }

  if(err) {
    Params::RangeQuerySelectRsp rsp(err);
    conn->send_response(Buffers::make(ev, rsp));
  } else {
    range->scan(
      std::make_shared<Ranger::Callback::RangeQuerySelect>(
        conn, ev, std::move(params.interval), range)
    );
  }
}


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeQuerySelect_h
