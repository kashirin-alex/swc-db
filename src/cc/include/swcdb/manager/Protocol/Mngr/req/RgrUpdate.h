
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_mngr_req_RgrUpdate_h
#define swc_manager_Protocol_mngr_req_RgrUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/RgrUpdate.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class RgrUpdate : public Comm::client::ConnQueue::ReqBase {
  public:

  RgrUpdate(const Manager::RangerList &hosts, bool sync_all)
            : Comm::client::ConnQueue::ReqBase(true) {
    cbp = Comm::Buffers::make(Params::RgrUpdate(hosts, sync_all));
    cbp->header.set(RGR_UPDATE, 60000);
  }
  
  virtual ~RgrUpdate() { }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    if(!is_rsp(ev))
      return;

    if(ev->response_code() != Error::OK)
      conn->do_close();
  }

};

}}}}

#endif // swc_manager_Protocol_mngr_req_RgrUpdate_h
