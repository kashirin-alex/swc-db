/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_fsbroker_handlers_Readdir_h
#define swc_fsbroker_handlers_Readdir_h

#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace FsBroker { namespace Handler {


void readdir(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  FS::DirentList results;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::ReaddirReq params;
    params.decode(&ptr, &remain);

    Env::FsInterface::fs()->readdir(err, params.dirname, results);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(FS::Protocol::Params::ReaddirRsp(results), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}


}}}

#endif // swc_fsbroker_handlers_Readdir_h