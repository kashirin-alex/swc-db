/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_Report_h
#define swcdb_ranger_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void report(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Buffers::Ptr cbp;

  try {
    
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    auto func(
      (Params::Report::Function)Serialization::decode_i8(&ptr, &remain));

    switch(func) {

      case Params::Report::Function::RESOURCES: {
        Params::Report::RspRes rsp_params;
        rsp_params.mem = Env::Rgr::res().available_mem_mb();
        rsp_params.cpu = Env::Rgr::res().available_cpu_mhz();

        rsp_params.ranges = 0;
        Ranger::ColumnPtr col;
        auto& columns = *Env::Rgr::columns();
        for(cid_t cidx = 0; (col=columns.get_next(cidx)); ++cidx) {
          rsp_params.ranges += col->ranges_count(); // *= (Master|Meta) weight
        }
        cbp = Buffers::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::CIDS: {
        Params::Report::RspCids rsp_params;
        Env::Rgr::columns()->get_cids(rsp_params.cids);

        cbp = Buffers::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMN_RIDS: {
        Params::Report::ReqColumn params;
        params.decode(&ptr, &remain);
        auto col = Env::Rgr::columns()->get_column(params.cid);
        if(!col) {
          err = Error::COLUMN_NOT_EXISTS;
          goto send_error;
        }

        Params::Report::RspColumnRids rsp_params;
        col->get_rids(rsp_params.rids);
  
        cbp = Buffers::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMN_RANGES: {
        Params::Report::ReqColumn params;
        params.decode(&ptr, &remain);
        
        auto rgr_data = Env::Rgr::rgr_data();
        rgrid_t rgrid;
        if(!(rgrid = rgr_data->rgrid)) {
          err = Error::RGR_NOT_READY;
          goto send_error;
        }

        auto col = Env::Rgr::columns()->get_column(params.cid);
        if(!col) {
          err = Error::COLUMN_NOT_EXISTS;
          goto send_error;
        }

        Params::Report::RspColumnsRanges rsp_params(
          rgrid, rgr_data->endpoints);

        auto c = new Params::Report::RspColumnsRanges::Column();
        rsp_params.columns.push_back(c);
        c->cid = col->cfg->cid;
        c->col_seq = col->cfg->key_seq;
        c->mem_bytes = 0;

        Ranger::RangePtr range;
        for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
          auto r = new Params::Report::RspColumnsRanges::Range(c->col_seq);
          c->ranges.push_back(r);
          c->mem_bytes += range->blocks.size_bytes_total(true);
          r->rid = range->rid;
          range->get_interval(r->interval);
        }
      
        cbp = Buffers::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      case Params::Report::Function::COLUMNS_RANGES: {

        auto rgr_data = Env::Rgr::rgr_data();
        rgrid_t rgrid;
        if(!(rgrid = rgr_data->rgrid)) {
          err = Error::RGR_NOT_READY;
          goto send_error;
        }

        Params::Report::RspColumnsRanges rsp_params(
          rgrid, rgr_data->endpoints);

        Ranger::ColumnPtr col;
        Ranger::RangePtr range;
        auto& columns = *Env::Rgr::columns();
        for(cid_t cidx = 0; (col=columns.get_next(cidx)); ++cidx) {
          auto c = new Params::Report::RspColumnsRanges::Column();
          rsp_params.columns.push_back(c);
          c->cid = col->cfg->cid;
          c->col_seq = col->cfg->key_seq;
          c->mem_bytes = 0;
          for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
            auto r = new Params::Report::RspColumnsRanges::Range(c->col_seq);
            c->ranges.push_back(r);
            c->mem_bytes += range->blocks.size_bytes_total(true);
            r->rid = range->rid;
            range->get_interval(r->interval);
          }
        }

        cbp = Buffers::make(rsp_params, 4);
        cbp->append_i32(err);
        goto send_response;
      }

      default: {
        err = Error::NOT_IMPLEMENTED;
        break;
      }
    
    }
    
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  
  send_error:
    cbp = Buffers::make(4);
    cbp->append_i32(err);


  send_response:
    try {
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_Report_h
