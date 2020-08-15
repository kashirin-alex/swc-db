
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

 
SWC_SHOULD_INLINE
void ColumnGet::schema(const std::string& name, const ColumnGet::Cb_t& cb, 
                        const uint32_t timeout) {
  request(Flag::SCHEMA_BY_NAME, name, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::schema(cid_t cid, const ColumnGet::Cb_t& cb,
                       const uint32_t timeout) {
  request(Flag::SCHEMA_BY_ID, cid, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::cid(const std::string& name, const ColumnGet::Cb_t& cb,
                    const uint32_t timeout) {
  request(Flag::ID_BY_NAME, name, cb, timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::request(ColumnGet::Flag flag, const std::string& name, 
                        const ColumnGet::Cb_t& cb,
                        const uint32_t timeout) {
  std::make_shared<ColumnGet>(Params::ColumnGetReq(flag, name), cb, timeout)
    ->run();
}

SWC_SHOULD_INLINE
void ColumnGet::request(ColumnGet::Flag flag, cid_t cid, 
                        const ColumnGet::Cb_t& cb, const uint32_t timeout) {
  std::make_shared<ColumnGet>(Params::ColumnGetReq(flag, cid), cb, timeout)
    ->run();
}


ColumnGet::ColumnGet(const Params::ColumnGetReq& params, 
                     const ColumnGet::Cb_t& cb, 
                     const uint32_t timeout) 
                    : client::ConnQueue::ReqBase(false), cb(cb) {
  cbp = CommBuf::make(params);
  cbp->header.set(COLUMN_GET, timeout);
}

ColumnGet::~ColumnGet() { }

void ColumnGet::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnGet::run() {
  if(endpoints.empty()) {
    // ColumnGet not like ColumnList (can be any mngr if by cid)
    Env::Clients::get()->mngrs_groups->select(
      Types::MngrRole::SCHEMAS, endpoints); 
    if(endpoints.empty()) {
      MngrActive::make(Types::MngrRole::SCHEMAS, shared_from_this())->run();
      return false;
    }
  } 
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnGet::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnGetRsp rsp_params;
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      rsp_params.decode(&ptr, &remain);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}

void ColumnGet::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}



}}}}