/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


ColumnList_Base::ColumnList_Base(
                      const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnListReq& params,
                      const uint32_t timeout)
                      : client::ConnQueue::ReqBase(
                          Buffers::make(params, 0, COLUMN_LIST, timeout)
                        ),
                        clients(clients) {
}

void ColumnList_Base::handle_no_conn() {
  if(clients->stopping()) {
    callback(Error::CLIENT_STOPPING, Params::ColumnListRsp());
  } else if(!valid()) {
    callback(Error::CANCELLED, Params::ColumnListRsp());
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnList_Base::run() {
  if(endpoints.empty()) {
    clients->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping()) {
        callback(Error::CLIENT_STOPPING, Params::ColumnListRsp());
      } else if(!valid()) {
        callback(Error::CANCELLED, Params::ColumnListRsp());
      } else {
        MngrActive::make(
          clients, DB::Types::MngrRole::SCHEMAS, shared_from_this())->run();
      }
      return false;
    }
  }
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ColumnList_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::ColumnListRsp rsp;
  int err = ev->response_code();
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      rsp.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  } else if(err == Error::REQUEST_TIMEOUT) {
    SWC_LOG_OUT(LOG_INFO, Error::print(SWC_LOG_OSTREAM, err); );
    request_again();
    return;
  }

  callback(err, rsp);
}

void ColumnList_Base::clear_endpoints() {
  clients->remove_mngr(endpoints);
  endpoints.clear();
}


}}}}}
