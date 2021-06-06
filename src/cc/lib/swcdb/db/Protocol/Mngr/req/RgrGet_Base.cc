/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


RgrGet_Base::RgrGet_Base(const Params::RgrGetReq& params,
                         const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  Buffers::make(params, 0 ,RGR_GET, timeout)
                ),
                cid(params.cid) {
}

void RgrGet_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    Params::RgrGetRsp rsp(Error::CLIENT_STOPPING);
    callback(rsp);
  } else if(!valid()) {
    Params::RgrGetRsp rsp(Error::CANCELLED);
    callback(rsp);
  } else {
    clear_endpoints();
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

void RgrGet_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::RgrGetRsp rsp_params(ev->error);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }

  callback(rsp_params);
}

void RgrGet_Base::clear_endpoints() {
  get_clients()->remove_mngr(endpoints);
  endpoints.clear();
}


}}}}}
