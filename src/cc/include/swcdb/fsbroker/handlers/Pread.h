/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Pread_h
#define swcdb_fsbroker_handlers_Pread_h

#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Handler {


void pread(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {

  int err = Error::OK;
  size_t offset = 0;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::PreadReq params;
    params.decode(&ptr, &remain);

    auto smartfd = Env::Fds::get()->select(params.fd);
      
    if(!smartfd) {
      err = EBADR;
    } else {
      offset = params.offset;
      rbuf.reallocate(params.amount);
      rbuf.size = Env::FsInterface::fs()->pread(
        err, smartfd, params.offset, rbuf.base, params.amount);
    }

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = Comm::Buffers::make(Params::ReadRsp(offset), rbuf, 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}
  

}}}}

#endif // swcdb_fsbroker_handlers_Pread_h