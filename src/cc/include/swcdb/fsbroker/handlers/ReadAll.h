/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_ReadAll_h
#define swcdb_fsbroker_handlers_ReadAll_h

#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Handler {


void read_all(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {

  int err = Error::OK;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ReadAllReq params;
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

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = err ? Comm::Buffers::make(4) : Comm::Buffers::make(rbuf, 4); 
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}


}}}}

#endif // swcdb_fsbroker_handlers_ReadAll_h