/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_Report_h
#define swcdb_ranger_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


struct Report {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  Report(const Comm::ConnHandlerPtr& a_conn,
         const Comm::Event::Ptr& a_ev) noexcept
        : conn(a_conn), ev(a_ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    int err = Error::OK;
    Buffers::Ptr cbp;

    try {

      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      switch(
        Params::Report::Function(Serialization::decode_i8(&ptr, &remain))) {

        case Params::Report::Function::RESOURCES: {
          cbp = Buffers::make(
            ev,
            Params::Report::RspRes(
              Env::Rgr::res().available_mem_mb(),
              Env::Rgr::res().available_cpu_mhz(),
              Env::Rgr::in_process_ranges()
            ),
            4
          );
          cbp->append_i32(err);
          goto send_response;
        }

        case Params::Report::Function::CIDS: {
          Params::Report::RspCids rsp_params;
          Env::Rgr::columns()->get_cids(rsp_params.cids);

          cbp = Buffers::make(ev, rsp_params, 4);
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

          cbp = Buffers::make(ev, rsp_params, 4);
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

          Core::Vector<Ranger::RangePtr> ranges;
          col->get_ranges(ranges);
          for(auto& range : ranges) {
            auto r = new Params::Report::RspColumnsRanges::Range(c->col_seq);
            c->ranges.push_back(r);
            c->mem_bytes += range->blocks.size_bytes_total(true);
            r->rid = range->rid;
            range->get_interval(r->interval);
          }

          cbp = Buffers::make(ev, rsp_params, 4);
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

          Core::Vector<Ranger::ColumnPtr> cols;
          Env::Rgr::columns()->get_columns(cols);
          Core::Vector<Ranger::RangePtr> ranges;
          for(auto& col : cols) {
            auto c = new Params::Report::RspColumnsRanges::Column();
            rsp_params.columns.push_back(c);
            c->cid = col->cfg->cid;
            c->col_seq = col->cfg->key_seq;
            c->mem_bytes = 0;
            col->get_ranges(ranges);
            for(auto& range : ranges) {
              auto r = new Params::Report::RspColumnsRanges::Range(
                c->col_seq);
              c->ranges.push_back(r);
              c->mem_bytes += range->blocks.size_bytes_total(true);
              r->rid = range->rid;
              range->get_interval(r->interval);
            }
            ranges.clear();
          }

          cbp = Buffers::make(ev, rsp_params, 4);
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
      cbp = Buffers::make(ev, 4);
      cbp->append_i32(err);


    send_response:
      conn->send_response(cbp);
  }

};


}}}}}

#endif // swcdb_ranger_Protocol_handlers_Report_h
