/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void ColumnList_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Error::CLIENT_STOPPING, Params::ColumnListRsp());
  } else if(!valid()) {
    callback(Error::CANCELLED, Params::ColumnListRsp());
  } else {
    get_clients()->remove_mngr(endpoints);
    endpoints.clear();
    run();
  }
}

void ColumnList_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::ColumnListRsp rsp;
  int err = ev->response_code();
  switch(err) {
    case Error::OK: {
      try {
        const uint8_t *ptr = ev->data.base + 4;
        size_t remain = ev->data.size - 4;
        rsp.decode(&ptr, &remain);

      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        err = e.code();
      }
      break;
    }
    case Error::MNGR_NOT_INITIALIZED :
    case Error::REQUEST_TIMEOUT: {
      SWC_LOG_OUT(LOG_INFO, Error::print(SWC_LOG_OSTREAM, err); );
      request_again();
      return;
    }
    default:
      return;
  }
  callback(err, rsp);
}



}}}}}
