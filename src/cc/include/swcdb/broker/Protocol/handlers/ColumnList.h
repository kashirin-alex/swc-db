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

  ConnHandlerPtr       conn;
  Event::Ptr           ev;
  Core::Atomic<size_t> remain;
  Core::StateRunning   processed;
  SWC_CAN_INLINE
  ColumnList(const ConnHandlerPtr& conn, const Event::Ptr& ev) noexcept
            : conn(conn), ev(ev), remain(0), processed(false) {
  }

  //~ColumnList() { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }

  SWC_CAN_INLINE
  bool valid() {
    return !ev->expired() && conn->is_open() && Env::Bkr::is_accepting();
  }

  SWC_CAN_INLINE
  void callback(const client::ConnQueue::ReqBase::Ptr&,
                int err, const Mngr::Params::ColumnListRsp& rsp) {
    bool last = true;
    if(!ev->expired() && conn->is_open()) {
      if(err == Error::CLIENT_STOPPING || !Env::Bkr::is_accepting())
        err = Error::SERVER_SHUTTING_DOWN;
      Buffers::Ptr cbp;
      if(err) {
        cbp = Buffers::make(ev, 4);
      } else {
        cbp = Buffers::make(ev, rsp, 4);
        if(rsp.expected) {
          uint64_t at = 0;
          remain.compare_exchange_weak(at, rsp.expected);
          if(remain.sub_rslt(rsp.schemas.size())) {
            cbp->header.flags |= Header::FLAG_RESPONSE_PARTIAL_BIT;
            last = false;
          }
        }
      }
      cbp->append_i32(err);
      conn->send_response(cbp);
    }
    if(!err)
      Env::Clients::get()->schemas.set(rsp.schemas);
    if(last && !processed.running())
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
