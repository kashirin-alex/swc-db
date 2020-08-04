/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_Protocol_handlers_Report_h
#define swc_ranger_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {


void report(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  CommBuf::Ptr cbp;

  try {
    Params::ReportReq params;
    const uint8_t* ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    auto rgr_data = RangerEnv::rgr_data();
    rgrid_t rgrid;
    if(!(rgrid = rgr_data->rgrid)) {
      Protocol::Rgr::Params::ReportRsp rsp_params(Error::RS_NOT_READY);
      cbp = CommBuf::make(rsp_params);
      goto send_response;
    }

    switch(params.flags) {
      case Params::ReportReq::COLUMN | Params::ReportReq::RANGES:
      case Params::ReportReq::COLUMN: {
        int err = Error::OK;
        auto col = RangerEnv::columns()->get_column(err, params.cid);
        Protocol::Rgr::Params::ReportRsp rsp_params(
          col ? err : Error::COLUMN_NOT_EXISTS);
        
        if(rsp_params.err) {      
          cbp = CommBuf::make(rsp_params);
          goto send_response;
        }
        
        rsp_params.rgrid = rgrid;
        rsp_params.endpoints.assign(
          rgr_data->endpoints.begin(), rgr_data->endpoints.end());

        auto c = new Protocol::Rgr::Params::ReportRsp::Column();
        rsp_params.columns.push_back(c);
        c->cid = col->cfg.cid;
        c->col_seq = col->cfg.key_seq;
        c->mem_bytes = 0;

        Ranger::RangePtr range;
        for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
          if(params.flags & Params::ReportReq::RANGES) {
            auto r = new Protocol::Rgr::Params::ReportRsp::Range(c->col_seq);
            c->ranges.push_back(r);
            r->rid = range->rid;
            range->get_interval(r->interval);
          }
          c->mem_bytes += range->blocks.size_bytes_total(true);
        }
        cbp = CommBuf::make(rsp_params);
        break;
      }

      case Params::ReportReq::COLUMNS:
      case Params::ReportReq::RANGES:
      case Params::ReportReq::COLUMNS | Params::ReportReq::RANGES: {
        Protocol::Rgr::Params::ReportRsp rsp_params(Error::OK);
        
        rsp_params.rgrid = rgrid;
        rsp_params.endpoints.assign(
          rgr_data->endpoints.begin(), rgr_data->endpoints.end());

        Ranger::Column::Ptr col;
        Ranger::RangePtr range;
        auto columns = RangerEnv::columns();
        for(cid_t cidx = 0; (col=columns->get_next(cidx)); ++cidx) {
          auto c = new Protocol::Rgr::Params::ReportRsp::Column();
          rsp_params.columns.push_back(c);
          c->cid = col->cfg.cid;
          c->col_seq = col->cfg.key_seq;
          c->mem_bytes = 0;
          for(rid_t ridx = 0; (range=col->get_next(ridx)); ++ridx) {
            if(params.flags & Params::ReportReq::RANGES) {
              auto r = new Protocol::Rgr::Params::ReportRsp::Range(c->col_seq);
              c->ranges.push_back(r);
              r->rid = range->rid;
              range->get_interval(r->interval);
            }
            c->mem_bytes += range->blocks.size_bytes_total(true);
          }
        }
        cbp = CommBuf::make(rsp_params);
        break;
      }

      case Params::ReportReq::RESOURCES: {
        Protocol::Rgr::Params::ReportResRsp rsp_params(Error::OK);
        rsp_params.mem = RangerEnv::res().available_mem_mb();
        rsp_params.cpu = RangerEnv::res().available_cpu_mhz();

        rsp_params.ranges = 0;
        Ranger::Column::Ptr col;
        auto columns = RangerEnv::columns();
        for(cid_t cidx = 0; (col=columns->get_next(cidx)); ++cidx) {
          rsp_params.ranges += col->ranges_count(); 
          // *= (Master | Meta) ratio
        }
        cbp = CommBuf::make(rsp_params);
        break;
      }

      default: {
        Protocol::Rgr::Params::ReportRsp rsp_params(Error::NOT_IMPLEMENTED);
        cbp = CommBuf::make(rsp_params);
        break;
      }
    }

  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    Protocol::Rgr::Params::ReportRsp rsp_params(e.code());
    cbp = CommBuf::make(rsp_params);
  }

  send_response:
  try{
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
  }
  
}
  

}}}}

#endif // swc_ranger_Protocol_handlers_Report_h