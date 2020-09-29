
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {



SWC_SHOULD_INLINE
void ColumnCompact::request(cid_t cid, const ColumnCompact::Cb_t& cb, 
                            const uint32_t timeout) {
  request(Params::ColumnCompactReq(cid), cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnCompact::request(const Params::ColumnCompactReq& params,
                            const ColumnCompact::Cb_t& cb, 
                            const uint32_t timeout) {
  std::make_shared<ColumnCompact>(params, cb, timeout)->run();
}


ColumnCompact::ColumnCompact(const Params::ColumnCompactReq& params, 
                             const ColumnCompact::Cb_t& cb, 
                             const uint32_t timeout) 
                            : Comm::client::ConnQueue::ReqBase(false), 
                              cb(cb), cid(params.cid) {
  cbp = Comm::Buffers::make(params);
  cbp->header.set(COLUMN_COMPACT, timeout);
}

ColumnCompact::~ColumnCompact() { }

void ColumnCompact::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnCompact::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(cid, endpoints); 
    if(endpoints.empty()) {
      MngrActive::make(cid, shared_from_this())->run();
      return false;
    }
  } 
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnCompact::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) {
  if(ev->type == Comm::Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnCompactRsp rsp_params(ev->error);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
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



}}}}
