/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Select/BrokerScanner.h"
#include "swcdb/db/Protocol/Bkr/req/CellsSelect.h"


namespace SWC { namespace client { namespace Query { namespace Select {



#define SWC_SCANNER_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_SCANNER_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );



void BrokerScanner::print(std::ostream& out) {
  out << "BrokerScanner(cid=" << cid << ' ';
  interval.print(out);
  out << " completion=" << selector->completion.count()
      << ')';
}

void BrokerScanner::select() {
  Comm::Protocol::Bkr::Params::CellsSelectReqRef params(cid, interval);
  SWC_SCANNER_REQ_DEBUG("bkr_select");
  struct ReqData {
    Ptr                         scanner;
    Profiling::Component::Start profile;
    SWC_CAN_INLINE
    ReqData(const Ptr& scanner) noexcept
            : scanner(scanner),
              profile(scanner->selector->profile.bkr()) {
    }
    SWC_CAN_INLINE
    client::Clients::Ptr& get_clients() noexcept {
      return scanner->selector->clients;
    }
    SWC_CAN_INLINE
    bool valid() {
      return scanner->selector->valid();
    }
    SWC_CAN_INLINE
    void callback(const ReqBase::Ptr& req,
                  Comm::Protocol::Bkr::Params::CellsSelectRsp& rsp) {
      profile.add(rsp.err);
      scanner->selected(req, rsp);
    }
  };
  Comm::Protocol::Bkr::Req::CellsSelect<ReqData>
    ::request(params, selector->timeout, shared_from_this());
}

void BrokerScanner::selected(const ReqBase::Ptr& req,
                             Comm::Protocol::Bkr::Params::CellsSelectRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_SCANNER_RSP_DEBUG("bkr_selected");
      if(interval.flags.offset)
        interval.flags.offset = rsp.offset;

      if(!rsp.data.size ||
         selector->add_cells(cid, rsp.data, rsp.more, interval)) {
        if(rsp.more && selector->valid())
          return select();
      }
      break;
    }
    case Error::COLUMN_MARKED_REMOVED:
    case Error::COLUMN_SCHEMA_NAME_NOT_EXISTS:
    case Error::COLUMN_NOT_EXISTS: {
      SWC_SCANNER_RSP_DEBUG("bkr_selected QUIT");
      selector->clients->schemas.remove(cid);
      selector->error(rsp.err); // rsp.err = Error::CONSIST_ERRORS;
      selector->error(cid, rsp.err);
      break;
    }
    case Error::CLIENT_STOPPING: {
      SWC_SCANNER_RSP_DEBUG("bkr_selected STOPPED");
      selector->error(rsp.err);
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
    selector->response(rsp.err);
}


#undef SWC_SCANNER_REQ_DEBUG
#undef SWC_SCANNER_RSP_DEBUG

}}}}
