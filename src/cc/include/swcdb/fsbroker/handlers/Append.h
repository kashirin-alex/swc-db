/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_fsbroker_handlers_Append_h
#define swc_fsbroker_handlers_Append_h

#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Handler {


void append(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {

  int err = Error::OK;
  size_t amount = 0;
  size_t offset = 0;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::AppendReq params;
    params.decode(&ptr, &remain);
      
    auto smartfd = Env::Fds::get()->select(params.fd);
      
    if(!smartfd) {
      err = EBADR;
    } else {
      offset = smartfd->pos();
      amount = Env::FsInterface::fs()->append(
        err, smartfd, ev->data_ext, (FS::Flags)params.flags);
    }

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  if(ev->expired())
    return;

  try {
    auto cbp = Comm::Buffers::make(Params::AppendRsp(offset, amount), 4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}


}}}}

#endif // swc_fsbroker_handlers_Append_h