/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_req_RgrUpdate_h
#define swcdb_manager_Protocol_mngr_req_RgrUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/RgrUpdate.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class RgrUpdate : public client::ConnQueue::ReqBase {
  public:

  RgrUpdate(const Manager::RangerList &hosts, bool sync_all)
            : client::ConnQueue::ReqBase(
                true,
                Buffers::make(
                  Params::RgrUpdate(hosts, sync_all), 0, RGR_UPDATE, 60000)
              ) {
  }

  virtual ~RgrUpdate() { }

  void handle_no_conn() override { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    if(ev->type != Event::Type::DISCONNECT &&
       ev->response_code() != Error::OK) {
      conn->do_close();
    }
  }

};

}}}}}

#endif // swcdb_manager_Protocol_mngr_req_RgrUpdate_h
