/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_fsbroker_handlers_Length_h
#define swc_fsbroker_handlers_Length_h

#include "swcdb/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace FsBroker { namespace Handler {


void length(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  size_t length = 0;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::LengthReq params;
    params.decode(&ptr, &remain);

    length = Env::FsInterface::fs()->length(err, params.fname);
      
  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(FS::Protocol::Params::LengthRsp(length), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}
  

}}}

#endif // swc_fsbroker_handlers_Length_h