
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

ColumnUpdate::ColumnUpdate(const Manager::Ranger::Ptr& rgr, 
                           const DB::Schema::Ptr& schema)
                          : client::ConnQueue::ReqBase(false), 
                            rgr(rgr), schema(schema) {
  cbp = Buffers::make(Params::ColumnUpdate(schema));
  cbp->header.set(SCHEMA_UPDATE, 60000);
}
  
ColumnUpdate::~ColumnUpdate() { }

void ColumnUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  updated(ev->response_code(), false);
}

void ColumnUpdate::handle_no_conn() {
  updated(Error::COMM_NOT_CONNECTED, true);
}

void ColumnUpdate::updated(int err, bool failure) {
  if(!err) {
    Env::Mngr::columns()->get_column(err, schema->cid)
                             ->change_rgr_schema(rgr->rgrid, schema->revision);
    if(!Env::Mngr::rangers()->update(schema, false)) {
      Env::Mngr::mngd_columns()->update(
        Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY,
        schema,
        err
      );
    }
  } else if(failure) {
    ++rgr->failures;
    request_again();
  } 
}


}}}}}
