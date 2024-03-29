/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_CAN_INLINE
ColumnUpdate::ColumnUpdate(const Manager::Ranger::Ptr& a_rgr,
                           const Manager::Column::Ptr& a_col,
                           const DB::Schema::Ptr& a_schema,
                           uint64_t a_req_id)
              : client::ConnQueue::ReqBase(
                  Buffers::make(
                    Params::ColumnUpdate(a_schema), 0, SCHEMA_UPDATE, 60000)
                ),
                rgr(a_rgr), col(a_col), schema(a_schema), req_id(a_req_id) {
}

void ColumnUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->response_code())
    return handle_no_conn();

  col->change_rgr_schema(rgr->rgrid, schema->revision);
  updated();
}

void ColumnUpdate::handle_no_conn() {
  if(rgr->state == DB::Types::MngrRangerState::ACK) {
    rgr->failures.fetch_add(1);
    request_again();
  } else {
    updated();
  }
}

void ColumnUpdate::updated() {
  if(!Env::Mngr::rangers()->update(col, schema, req_id, false)) {
    Env::Mngr::mngd_columns()->update(
      Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY,
      schema,
      Error::OK,
      req_id
    );
  }
}


}}}}}
