/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Mkdirs_h
#define swcdb_fsbroker_handlers_Mkdirs_h

#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Handler {


void mkdirs(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {

  int err = Error::OK;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::MkdirsReq params;
    params.decode(&ptr, &remain);

    Env::FsInterface::fs()->mkdirs(err, params.dirname);

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  if(ev->expired())
    return;

  try {
    auto cbp = Comm::Buffers::make(4);
    cbp->header.initialize_from_request_header(ev->header);
    cbp->append_i32(err);
    conn->send_response(cbp);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  
}
  

}}}}

#endif // swcdb_fsbroker_handlers_Mkdirs_h