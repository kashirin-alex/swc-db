/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnList::request(ColumnList::Cb_t&& cb,
                         const uint32_t timeout) {
  request(Params::ColumnListReq(), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnList::request(const Params::ColumnListReq& params,
                         ColumnList::Cb_t&& cb,
                         const uint32_t timeout) {
  std::make_shared<ColumnList>(params, std::move(cb), timeout)->run();
}

ColumnList::ColumnList(const Params::ColumnListReq& params,
                       ColumnList::Cb_t&& cb,
                       const uint32_t timeout)
                      : client::ConnQueue::ReqBase(
                          false,
                          Buffers::make(params, 0, COLUMN_LIST, timeout)
                        ),
                        cb(std::move(cb)) {
}

void ColumnList::handle_no_conn() {
  if(Env::Clients::get()->stopping()) {
    cb(req(), Error::CLIENT_STOPPING, Params::ColumnListRsp());
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnList::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(
      DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(Env::Clients::get()->stopping()) {
        cb(req(), Error::CLIENT_STOPPING, Params::ColumnListRsp());
      } else {
        MngrActive::make(
          DB::Types::MngrRole::SCHEMAS, shared_from_this())->run();
      }
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnList::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnListRsp rsp_params;
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}

void ColumnList::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}


}}}}}
