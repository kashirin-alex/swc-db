/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
RangeLoad::RangeLoad(const Manager::Ranger::Ptr& a_rgr,
                     const Manager::Column::Ptr& a_col,
                     const Manager::Range::Ptr& a_range,
                     const DB::Schema::Ptr& schema)
        : client::ConnQueue::ReqBase(
            Buffers::make(
              Params::RangeLoad(schema, a_range->rid),
              0,
              RANGE_LOAD, 3600000
            )
          ),
          rgr(a_rgr), col(a_col), range(a_range),
          schema_revision(schema->revision) {
  SWC_LOG_OUT(LOG_INFO, range->print(SWC_LOG_OSTREAM  << "RANGE-LOAD "); );
}

void RangeLoad::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(!valid())
    return handle_no_conn();

  Params::RangeLoaded params(range->cfg->key_seq);
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }
  loaded(err, false, params.interval, params.revision);
}

bool RangeLoad::valid() {
  return !range->deleted() &&
          Env::Mngr::rangers()->running();
}

void RangeLoad::handle_no_conn() {
  loaded(Error::COMM_NOT_CONNECTED, true,
         DB::Cells::Interval(range->cfg->key_seq), 0);
}


void RangeLoad::loaded(int err, bool failure,
                       const DB::Cells::Interval& intval, int64_t revision) {
  if(!err)
    col->change_rgr_schema(rgr->rgrid, schema_revision);

  Env::Mngr::rangers()->range_loaded(
    rgr, range, revision, err, failure, false);
  col->sort(range, intval, revision);

  SWC_LOG_OUT(LOG_INFO,
    Error::print(SWC_LOG_OSTREAM << "RANGE-STATUS ", err);
    range->print(SWC_LOG_OSTREAM << ", ");
  );
}


}}}}}
