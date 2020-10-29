/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RangeRemove_h
#define swcdb_manager_Protocol_handlers_RangeRemove_h

#include "swcdb/db/Protocol/Mngr/params/RangeRemove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void range_remove(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::RangeRemoveRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeRemoveReq params;
    params.decode(&ptr, &remain);
    SWC_PRINT << "RangeRemove: " << params.to_string() << SWC_PRINT_CLOSE;

    auto col = Env::Mngr::mngd_columns()->get_column(
      rsp_params.err, params.cid);
    if(rsp_params.err) {
      if(rsp_params.err == Error::COLUMN_MARKED_REMOVED ||
         rsp_params.err == Error::MNGR_NOT_ACTIVE ||
         rsp_params.err == Error::COLUMN_NOT_EXISTS)
        goto send_response;
      rsp_params.err = Error::OK;
    }

    col->remove_range(params.rid);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }
  
  send_response:
    SWC_PRINT << "RangeRemove(RSP): " << rsp_params.to_string()
              << SWC_PRINT_CLOSE;
    conn->send_response(Buffers::make(rsp_params), ev);

}


}}}}}

#endif // swcdb_manager_Protocol_handlers_RangeRemove_h
