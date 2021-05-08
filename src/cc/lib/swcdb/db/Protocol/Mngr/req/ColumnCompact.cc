/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



SWC_SHOULD_INLINE
void ColumnCompact::request(cid_t cid, ColumnCompact::Cb_t&& cb,
                            const uint32_t timeout) {
  request(Params::ColumnCompactReq(cid), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnCompact::request(const Params::ColumnCompactReq& params,
                            ColumnCompact::Cb_t&& cb,
                            const uint32_t timeout) {
  std::make_shared<ColumnCompact>(params, std::move(cb), timeout)->run();
}


ColumnCompact::ColumnCompact(const Params::ColumnCompactReq& params,
                             ColumnCompact::Cb_t&& cb,
                             const uint32_t timeout)
                            : client::ConnQueue::ReqBase(
                                false,
                                Buffers::make(
                                  params, 0,
                                  COLUMN_COMPACT, timeout
                                )
                              ),
                              cb(std::move(cb)), cid(params.cid) {
}

void ColumnCompact::handle_no_conn() {
  if(Env::Clients::get()->stopping()) {
    cb(req(), Params::ColumnCompactRsp(Error::CLIENT_STOPPING));
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnCompact::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(cid, endpoints);
    if(endpoints.empty()) {
      if(Env::Clients::get()->stopping()) {
        cb(req(), Params::ColumnCompactRsp(Error::CLIENT_STOPPING));
      } else {
        MngrActive::make(cid, shared_from_this())->run();
      }
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnCompact::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnCompactRsp rsp_params(ev->error);
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

  cb(req(), rsp_params);
}

void ColumnCompact::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}



}}}}}
