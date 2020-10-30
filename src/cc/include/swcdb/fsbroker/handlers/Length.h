/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Length_h
#define swcdb_fsbroker_handlers_Length_h

#include "swcdb/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void length(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  size_t length = 0;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::LengthReq params;
    params.decode(&ptr, &remain);

    length = Env::FsInterface::fs()->length(err, params.fname);
      
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  
  auto cbp = Buffers::make(ev, Params::LengthRsp(length), 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}
  

}}}}}

#endif // swcdb_fsbroker_handlers_Length_h
