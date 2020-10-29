/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_ReadAll_h
#define swcdb_fsbroker_handlers_ReadAll_h

#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void read_all(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

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
    fd = Env::FsBroker::fds()->add(smartfd);

    rbuf.free();
    if(fs->read(err, smartfd, &rbuf, len) != len)
      err = Error::FS_EOF;

    finish:
      int errtmp;
      if(fd != -1 && (smartfd = Env::FsBroker::fds()->remove(fd)))
        fs->close(!err ? err : errtmp, smartfd);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  auto cbp = err ? Buffers::make(4) : Buffers::make(rbuf, 4);
  cbp->append_i32(err);
  conn->send_response(cbp, ev);

}


}}}}}

#endif // swcdb_fsbroker_handlers_ReadAll_h
