/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RangeUnloaded_h
#define swcdb_manager_Protocol_handlers_RangeUnloaded_h

#include "swcdb/db/Protocol/Mngr/params/RangeUnloaded.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void range_unloaded(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::RangeUnloadedRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::RangeUnloadedReq params;
    params.decode(&ptr, &remain);
    SWC_PRINT << "RangeUnloaded: " << params.to_string() << SWC_PRINT_CLOSE;

    auto col = Env::Mngr::mngd_columns()->get_column(
      rsp_params.err, params.cid);
    if(rsp_params.err) {
      if(rsp_params.err == Error::COLUMN_MARKED_REMOVED ||
         rsp_params.err == Error::MNGR_NOT_ACTIVE ||
         rsp_params.err == Error::COLUMN_NOT_EXISTS)
        goto send_response;
      rsp_params.err = Error::OK;
    }

    auto range = col->get_range(params.rid);
    if(!range) {
      rsp_params.err = Error::RANGE_NOT_FOUND;
      goto send_response;
    }
    range->set_state(Manager::Range::State::NOTSET, 0);

    Env::Mngr::rangers()->schedule_check(1);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }

  send_response:
    SWC_PRINT << "RangeUnloaded(RSP): " << rsp_params.to_string()
              << SWC_PRINT_CLOSE;
    conn->send_response(Buffers::make(ev, rsp_params));

}


}}}}}

#endif // swcdb_manager_Protocol_handlers_RangeUnloaded_h
