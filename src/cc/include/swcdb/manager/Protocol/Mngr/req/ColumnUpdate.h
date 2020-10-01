
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_mngr_req_ColumnUpdate_h
#define swcdb_manager_Protocol_mngr_req_ColumnUpdate_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/manager/Protocol/Mngr/params/ColumnUpdate.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

class ColumnUpdate : public Comm::client::ConnQueue::ReqBase {
  public:

  ColumnUpdate(Params::ColumnMng::Function function, DB::Schema::Ptr schema, 
               int err) : Comm::client::ConnQueue::ReqBase(true) {
    cbp = Comm::Buffers::make(Params::ColumnUpdate(function, schema, err));
    cbp->header.set(COLUMN_UPDATE, 60000);
  }
  
  virtual ~ColumnUpdate() { }
  
  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    if(!is_rsp(ev))
      return;

    if(ev->response_code() != Error::OK)
      conn->do_close();
  }
  
};

}}}}

#endif // swcdb_manager_Protocol_mngr_req_ColumnUpdate_h
