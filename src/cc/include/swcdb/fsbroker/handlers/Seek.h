/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_fsbroker_handlers_Seek_h
#define swc_fsbroker_handlers_Seek_h

#include "swcdb/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace FsBroker { namespace Handler {


void seek(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  size_t offset = 0;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::SeekReq params;
    params.decode(&ptr, &remain);

    auto smartfd = Env::Fds::get()->select(params.fd);
      
    if(!smartfd) {
      err = EBADR;
    } else {
      Env::FsInterface::fs()->seek(err, smartfd, params.offset);
      offset = smartfd->pos();
    }

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = CommBuf::make(FS::Protocol::Params::SeekRsp(offset), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
    
}
  

}}}

#endif // swc_fsbroker_handlers_Seek_h