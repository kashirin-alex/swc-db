/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void RgrGet_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    Params::RgrGetRsp rsp(Error::CLIENT_STOPPING);
    callback(rsp);
  } else if(!valid()) {
    Params::RgrGetRsp rsp(Error::CANCELLED);
    callback(rsp);
  } else {
    get_clients()->remove_mngr(endpoints);
    endpoints.clear();
    run();
  }
}

bool RgrGet_Base::run() {
  if(endpoints.empty()) {
    get_clients()->get_mngr(cid, endpoints);
    if(endpoints.empty()) {
      if(get_clients()->stopping()) {
        Params::RgrGetRsp rsp(Error::CLIENT_STOPPING);
        callback(rsp);
      } else if(!valid()) {
        Params::RgrGetRsp rsp(Error::CANCELLED);
        callback(rsp);
      } else {
        SWC_LOGF(LOG_DEBUG, "RgrGet req mngr-active for cid=%lu", cid);
        MngrActive::make(get_clients(), cid, shared_from_this())->run();
      }
      return false;
    }
  }
  get_clients()->get_mngr_queue(endpoints)->put(req());
  return true;
}



}}}}}
