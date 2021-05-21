/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Select/BrokerScanner.h"
#include "swcdb/db/Protocol/Bkr/req/Scanner_CellsSelect.h"


namespace SWC { namespace client { namespace Query { namespace Select {



#define SWC_SCANNER_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_SCANNER_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );



BrokerScanner::BrokerScanner(const Handlers::Base::Ptr& hdlr,
                             const DB::Specs::Interval& interval,
                             const cid_t cid)
                            : selector(hdlr), interval(interval), cid(cid) {
}

void BrokerScanner::print(std::ostream& out) {
  out << "BrokerScanner(cid" << cid << ' ';
  interval.print(out);
  out << " completion=" << selector->completion.count()
      << ')';
}

bool BrokerScanner::add_cells(StaticBuffer& buffer, bool reached_limit) {
  return selector->add_cells(cid, buffer, reached_limit, interval);
}

void BrokerScanner::select() {
  selector->completion.increment();
  Comm::Protocol::Bkr::Params::CellsSelectReq params(cid, interval);
  SWC_SCANNER_REQ_DEBUG("bkr_select");
  Comm::Protocol::Bkr::Req::Scanner_CellsSelect::request(
    shared_from_this(), params);
}

void BrokerScanner::selected(const ReqBase::Ptr& req,
                             Comm::Protocol::Bkr::Params::CellsSelectRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_SCANNER_RSP_DEBUG("bkr_selected");
      if(interval.flags.offset)
        interval.flags.offset = rsp.offset;

      if(!rsp.data.size || add_cells(rsp.data, rsp.reached_limit))
        if(rsp.reached_limit && selector->valid())
          select();
      break;
    }
    default: {
      if(selector->valid()) {
        SWC_SCANNER_RSP_DEBUG("bkr_selected RETRYING");
        return req->request_again();
      }
    }
  }
  if(selector->completion.is_last())
    selector->response(Error::OK);
}


#undef SWC_SCANNER_REQ_DEBUG
#undef SWC_SCANNER_RSP_DEBUG

}}}}
