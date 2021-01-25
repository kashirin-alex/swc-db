/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Sync_h
#define swcdb_fsbroker_handlers_Sync_h

#include "swcdb/fs/Broker/Protocol/params/Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void sync(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::SyncReq params;
    params.decode(&ptr, &remain);

    auto smartfd = Env::FsBroker::fds().select(params.fd);

    if(!smartfd)
      err = EBADR;
    else
      Env::FsInterface::fs()->sync(err, smartfd);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = Buffers::make(ev, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}


}}}}}

#endif // swcdb_fsbroker_handlers_Sync_h
