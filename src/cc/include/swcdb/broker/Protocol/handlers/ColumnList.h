/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_ColumnList_h
#define swcdb_broker_Protocol_handlers_ColumnList_h


#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


struct ColumnList {

  ConnHandlerPtr conn;
  Event::Ptr     ev;

  SWC_CAN_INLINE
  ColumnList(const ConnHandlerPtr& conn, const Event::Ptr& ev) noexcept
            : conn(conn), ev(ev) {
  }

  //~ColumnList() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }

  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&,
                int err, const Mngr::Params::ColumnListRsp& rsp) {
    if(valid()) {
      auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rsp, 4);
      cbp->append_i32(
        err == Error::CLIENT_STOPPING ? Error::SERVER_SHUTTING_DOWN : err);
      conn->send_response(cbp);
    }
    if(!err)
      Env::Clients::get()->schemas.set(rsp.schemas);
    Env::Bkr::processed();
  }

};


void column_list(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Mngr::Params::ColumnListReq params;
    params.decode(&ptr, &remain);

    Mngr::Req::ColumnList<ColumnList>::request(
      params, ev->header.timeout_ms, conn, ev);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );

    auto cbp =  Buffers::make(ev, 4);
    cbp->append_i32(e.code());
    conn->send_response(cbp);
    Env::Bkr::processed();
  }
}



}}}}}

#endif // swcdb_broker_Protocol_handlers_ColumnList_h
