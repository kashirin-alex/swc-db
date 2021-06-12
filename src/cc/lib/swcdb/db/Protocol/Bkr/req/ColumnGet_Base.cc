/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/ColumnGet_Base.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {



void ColumnGet_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Error::CLIENT_STOPPING, Mngr::Params::ColumnGetRsp());
  } else if(!valid()) {
    callback(Error::CANCELLED, Mngr::Params::ColumnGetRsp());
  } else if(_bkr_idx.turn_around(get_clients()->brokers)) {
    request_again();
  } else {
    run();
  }
}

void ColumnGet_Base::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Mngr::Params::ColumnGetRsp rsp;
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
