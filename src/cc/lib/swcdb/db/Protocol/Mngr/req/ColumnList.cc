
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnList::request(const ColumnList::Cb_t& cb, const uint32_t timeout) {
  std::make_shared<ColumnList>(Params::ColumnListReq(), cb, timeout)->run();
}

ColumnList::ColumnList(const Params::ColumnListReq& params, 
                       const ColumnList::Cb_t& cb, const uint32_t timeout) 
                      : client::ConnQueue::ReqBase(false), cb(cb) {
  cbp = CommBuf::make(params);
  cbp->header.set(COLUMN_LIST, timeout);
}

ColumnList::~ColumnList() { }

void ColumnList::handle_no_conn() {
  clear_endpoints();
  run();
}

bool ColumnList::run(uint32_t timeout) {
  if(endpoints.empty()) {
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

void ColumnList::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {

  if(ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  Params::ColumnListRsp rsp_params;
  int err = ev->error != Error::OK? ev->error: ev->response_code();

  if(err == Error::OK) {
    try{
      const uint8_t *ptr = ev->data.base+4;
      size_t remain = ev->data.size-4;
      rsp_params.decode(&ptr, &remain);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}

void ColumnList::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}
