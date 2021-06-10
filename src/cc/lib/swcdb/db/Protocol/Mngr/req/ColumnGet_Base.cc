/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Base.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void ColumnGet_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Error::CLIENT_STOPPING, Params::ColumnGetRsp());
  } else if(!valid()) {
    callback(Error::CANCELLED, Params::ColumnGetRsp());
  } else {
    get_clients()->remove_mngr(endpoints);
    endpoints.clear();
    run();
  }
}

bool ColumnGet_Base::run() {
  if(endpoints.empty()) {
    // ColumnGet not like ColumnList (can be any mngr if by cid)
    get_clients()->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(get_clients()->stopping()) {
        callback(Error::CLIENT_STOPPING, Params::ColumnGetRsp());
      } else if(!valid()) {
        callback(Error::CANCELLED, Params::ColumnGetRsp());
      } else {
        MngrActive::make(
          get_clients(), DB::Types::MngrRole::SCHEMAS, shared_from_this()
        )->run();
      }
      return false;
    }
  }
  get_clients()->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ColumnGet_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::ColumnGetRsp rsp;
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



}}}}}
