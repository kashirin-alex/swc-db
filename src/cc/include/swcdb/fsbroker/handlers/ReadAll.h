/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_fsbroker_handlers_ReadAll_h
#define swc_fsbroker_handlers_ReadAll_h

#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace FsBroker { namespace Handler {


void read_all(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  size_t offset = 0;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    FS::Protocol::Params::ReadAllReq params;
    params.decode(&ptr, &remain);

    //Env::FsInterface::fs()->read(err, params.name, &rbuf); needs fds state

    FS::SmartFd::Ptr smartfd;
    int32_t fd = -1;
    size_t len;
    auto fs = Env::FsInterface::fs();
    if(!fs->exists(err, params.name)) {
      if(!err)
        err = Error::FS_PATH_NOT_FOUND;
      goto finish;
    } 
    
    len = fs->length(err, params.name);
    if(err)
      goto finish;

    fs->open(err, smartfd = FS::SmartFd::make_ptr(params.name, 0));
    if(!err && !smartfd->valid())
      err = EBADR;
    if(err)
      goto finish;
    fd = Env::Fds::get()->add(smartfd);

    rbuf.free();
    if(fs->read(err, smartfd, &rbuf, len) != len)
      err = Error::FS_EOF;

    finish:
      int errtmp;
      if(fd != -1 && (smartfd = Env::Fds::get()->remove(fd)))
        fs->close(!err ? err : errtmp, smartfd);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = err ? CommBuf::make(4) : CommBuf::make(rbuf, 4); 
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }

}


}}}

#endif // swc_fsbroker_handlers_ReadAll_h