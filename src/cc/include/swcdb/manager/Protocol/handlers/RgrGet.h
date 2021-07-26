/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_RgrGet_h
#define swcdb_manager_Protocol_handlers_RgrGet_h

#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


void rgr_get(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  Params::RgrGetReq params;
  Params::RgrGetRsp rsp_params;
  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    auto col = Env::Mngr::mngd_columns()->get_column(
      rsp_params.err, params.cid);
    if(rsp_params.err)
      goto send_response;

    //if(params.had_err == Error::RGR_NOT_LOADED_RANGE) / tick-check
    //  Env::Mngr::rangers()->need_health_check(col);

    Manager::Range::Ptr range;
    if(!params.rid) { // MASTER-COLUMN
      range = col->get_range(
        rsp_params.err,
        params.range_begin,
        params.range_end,
        params.next_range
      );
      if(range) {
        range->get_interval(
          rsp_params.range_begin, rsp_params.range_end,
          rsp_params.revision
        );
      }
    } else {
      range = col->get_range(params.rid);
    }

    if(!range) {
      rsp_params.err = Error::RANGE_NOT_FOUND;
      goto send_response;
    }

    Env::Mngr::rangers()->rgr_get(
      range->get_rgr_id(), rsp_params.endpoints);
    if(rsp_params.endpoints.empty()) {
      rsp_params.err = Error::RGR_NOT_READY;
      goto send_response;
    }

    rsp_params.cid = range->cfg->cid;
    rsp_params.rid = range->rid;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    rsp_params.err = e.code();
  }

  send_response:

    SWC_LOG_OUT(LOG_DEBUG,
      params.print(SWC_LOG_OSTREAM);
      rsp_params.print(SWC_LOG_OSTREAM <<' ');
    );

    conn->send_response(Buffers::make(ev, rsp_params));

}


// flag(if cid==1)
//      in(cid+interval)  out(cid + rid + rgr-endpoints + ?next)
// else
//      in(cid+rid)        out(cid + rid + rgr-endpoints)

}}}}}

#endif // swcdb_manager_Protocol_handlers_RgrGet_h
