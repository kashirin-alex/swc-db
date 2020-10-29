/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_ColumnCompact_h
#define swcdb_ranger_Protocol_handlers_ColumnCompact_h

#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void column_compact(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::ColumnCompactRsp rsp_params;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::ColumnCompactReq params;
    params.decode(&ptr, &remain);

    auto col = Env::Rgr::columns()->get_column(params.cid);
    if(col)
      col->compact();

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  
  conn->send_response(Buffers::make(rsp_params), ev);

}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_ColumnCompact_h
