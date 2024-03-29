/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnDelete.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
ColumnDelete::ColumnDelete(const Manager::Ranger::Ptr& a_rgr,
                           const DB::Schema::Ptr& a_schema,
                           uint64_t a_req_id)
          : client::ConnQueue::ReqBase(
              Buffers::make(
                Params::ColumnDelete(a_schema->cid),
                0,
                COLUMN_DELETE, 3600000)
            ),
            rgr(a_rgr), schema(a_schema), req_id(a_req_id) {
}

void ColumnDelete::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->response_code())
    return handle_no_conn();

  remove();
}

void ColumnDelete::handle_no_conn() {
  if(rgr->state == DB::Types::MngrRangerState::ACK) {
    rgr->failures.fetch_add(1);
    request_again();
  } else {
    remove();
  }
}

SWC_CAN_INLINE
void ColumnDelete::remove() {
  Env::Mngr::mngd_columns()->remove(schema, rgr->rgrid, req_id);
}

}}}}}
