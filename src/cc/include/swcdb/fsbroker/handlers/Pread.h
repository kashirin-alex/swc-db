/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Pread_h
#define swcdb_fsbroker_handlers_Pread_h

#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void pread(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  size_t offset = 0;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::PreadReq params;
    params.decode(&ptr, &remain);

    auto smartfd = Env::FsBroker::fds().select(params.fd);

    if(!smartfd) {
      err = EBADR;
    } else {
      offset = params.offset;
      rbuf.reallocate(params.amount);
      rbuf.size = Env::FsInterface::fs()->pread(
        err, smartfd, params.offset, rbuf.base, params.amount);
    }

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = Buffers::make(ev, Params::ReadRsp(offset), rbuf, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}


}}}}}

#endif // swcdb_fsbroker_handlers_Pread_h
