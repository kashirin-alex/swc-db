/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_fsbroker_handlers_Exists_h
#define swc_fsbroker_handlers_Exists_h

#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace FsBroker { namespace Handler {


void exists(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  bool exists = false;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::ExistsReq params;
    params.decode(&ptr, &remain);

    exists = Env::FsInterface::fs()->exists(err, params.fname);
      
  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(FS::Protocol::Params::ExistsRsp(exists), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}
  

}}}

#endif // swc_fsbroker_handlers_Exists_h