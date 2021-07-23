/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeLocate_h
#define swcdb_ranger_Protocol_handlers_RangeLocate_h

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/ranger/callbacks/RangeLocateScanCommit.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


struct RangeLocate {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  RangeLocate(const Comm::ConnHandlerPtr& conn,
              const Comm::Event::Ptr& ev) noexcept
              : conn(conn), ev(ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    int err = Error::OK;
    Params::RangeLocateReq params;
    Ranger::RangePtr range;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      params.decode(&ptr, &remain);

      range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);
      if((!err && (!range || !range->is_loaded())) ||
         (params.flags & Params::RangeLocateReq::HAVE_REVISION &&
          params.revision != range->get_load_revision()))
        err = Error::RGR_NOT_LOADED_RANGE;

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }


    if(err) {
      SWC_LOG_OUT(LOG_DEBUG,
        params.print(SWC_LOG_OSTREAM);
        Error::print(SWC_LOG_OSTREAM << ' ', err);
      );

      conn->send_response(Buffers::make(ev, Params::RangeLocateRsp(err)));

    } else {

      Ranger::ReqScan* req;
      if(params.flags & Params::RangeLocateReq::COMMIT) {
        req = new Ranger::Callback::RangeLocateScanCommit(
          conn, ev,
          params.range_begin, //params.range_end,
          range,
          params.flags
        );
      } else {
        req = new Ranger::Callback::RangeLocateScan(
          conn, ev,
          params.range_begin, params.range_end,
          range,
          params.flags
        );
        if(params.flags & Params::RangeLocateReq::NEXT_RANGE ||
           params.flags & Params::RangeLocateReq::CURRENT_RANGE)
          req->spec.offset_key.copy(params.range_offset);
      }

      range->scan(Ranger::ReqScan::Ptr(req));
    }

  }

};


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeLocate_h
