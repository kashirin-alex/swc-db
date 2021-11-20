/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_CombiPread_h
#define swcdb_fsbroker_handlers_CombiPread_h

#include "swcdb/fs/Broker/Protocol/params/CombiPread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void combi_pread(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::CombiPreadReq params;
    params.decode(&ptr, &remain);

    Env::FsInterface::fs()->combi_pread(
      err, params.smartfd,
      params.offset, params.amount,
      &rbuf
    );

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, rbuf, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}


}}}}}

#endif // swcdb_fsbroker_handlers_CombiPread_h
