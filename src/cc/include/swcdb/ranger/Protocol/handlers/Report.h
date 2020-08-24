/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_Protocol_handlers_Report_h
#define swc_ranger_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void report(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  CommBuf::Ptr cbp;

  try {
    
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    auto func(
      (Params::Report::Function)Serialization::decode_i8(&ptr, &remain));

    switch(func) {

      case Params::Report::Function::RESOURCES: {
        Params::Report::RspRes rsp_params;
        rsp_params.mem = RangerEnv::res().available_mem_mb();
        rsp_params.cpu = RangerEnv::res().available_cpu_mhz();

        rsp_params.ranges = 0;
        Ranger::Column::Ptr col;
        auto& columns = *RangerEnv::columns();
        for(cid_t cidx = 0; (col=columns.get_next(cidx)); ++cidx) {
          rsp_params.ranges += col->ranges_count(); // *= (Master|Meta) weight
        }
        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::CIDS: {
        Params::Report::RspCids rsp_params;
        RangerEnv::columns()->get_cids(rsp_params.cids);

        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMN_RIDS: {
        Params::Report::ReqColumn params;
        params.decode(&ptr, &remain);
        auto col = RangerEnv::columns()->get_column(err, params.cid);
        if(!col)
          err = Error::COLUMN_NOT_EXISTS;
        if(err)
          goto send_error;

        Params::Report::RspColumnRids rsp_params;
        col->get_rids(rsp_params.rids);
  
        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMN_RANGES: {
        Params::Report::ReqColumn params;
        params.decode(&ptr, &remain);
        
        auto rgr_data = RangerEnv::rgr_data();
        rgrid_t rgrid;
        if(!(rgrid = rgr_data->rgrid)) {
          err = Error::RS_NOT_READY;
          goto send_error;
        }

        auto col = RangerEnv::columns()->get_column(err, params.cid);
        if(!col)
          err = Error::COLUMN_NOT_EXISTS;
        if(err)
          goto send_error;

        Params::Report::RspColumnsRanges rsp_params(
          rgrid, rgr_data->endpoints);

        auto c = new Params::Report::RspColumnsRanges::Column();
        rsp_params.columns.push_back(c);
        c->cid = col->cfg.cid;
        c->col_seq = col->cfg.key_seq;
        c->mem_bytes = 0;

        Ranger::RangePtr range;
        for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
          auto r = new Params::Report::RspColumnsRanges::Range(c->col_seq);
          c->ranges.push_back(r);
          c->mem_bytes += range->blocks.size_bytes_total(true);
          r->rid = range->rid;
          range->get_interval(r->interval);
        }
      
        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMNS_RANGES: {

        auto rgr_data = RangerEnv::rgr_data();
        rgrid_t rgrid;
        if(!(rgrid = rgr_data->rgrid)) {
          err = Error::RS_NOT_READY;
          goto send_error;
        }

        Params::Report::RspColumnsRanges rsp_params(
          rgrid, rgr_data->endpoints);

        Ranger::Column::Ptr col;
        Ranger::RangePtr range;
        auto& columns = *RangerEnv::columns();
        for(cid_t cidx = 0; (col=columns.get_next(cidx)); ++cidx) {
          auto c = new Params::Report::RspColumnsRanges::Column();
          rsp_params.columns.push_back(c);
          c->cid = col->cfg.cid;
          c->col_seq = col->cfg.key_seq;
          c->mem_bytes = 0;
          for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
            auto r = new Params::Report::RspColumnsRanges::Range(c->col_seq);
            c->ranges.push_back(r);
            c->mem_bytes += range->blocks.size_bytes_total(true);
            r->rid = range->rid;
            range->get_interval(r->interval);
          }
        }

        cbp = CommBuf::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      default: {
        err = Error::NOT_IMPLEMENTED;
        break;
      }
    
    }
    
  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    err = e.code();
  }

  
  send_error:
    cbp = CommBuf::make(4);
    cbp->append_i32(err);


  send_response:
    try {
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_Report_h