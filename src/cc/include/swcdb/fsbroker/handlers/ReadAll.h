/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_ReadAll_h
#define swcdb_fsbroker_handlers_ReadAll_h

#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


namespace {
void send_response(const ConnHandlerPtr& conn, const Event::Ptr& ev,
                   int err, StaticBuffer& buffer) {
  auto cbp = err ? Buffers::make(ev, 4) : Buffers::make(ev, buffer, 4);
  cbp->append_i32(err);
  conn->send_response(cbp);
}
}


void read_all(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  StaticBuffer rbuf;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ReadAllReq params;
    params.decode(&ptr, &remain);

    const auto& fs = Env::FsInterface::fs();

    if(fs->impl_options.has_async_readall()) {
      fs->read(
        [conn=conn, ev=ev](int _err, const StaticBuffer::Ptr& buffer) {
          send_response(conn, ev, _err, *buffer.get());
        },
        params.name
      );
      return;
    }

    fs->read(err, params.name, &rbuf);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  send_response(conn, ev, err, rbuf);

}


}}}}}

#endif // swcdb_fsbroker_handlers_ReadAll_h
