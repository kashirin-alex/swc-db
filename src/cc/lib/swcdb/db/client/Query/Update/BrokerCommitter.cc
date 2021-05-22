/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/BrokerCommitter.h"
#include "swcdb/db/Protocol/Bkr/req/Committer_CellsUpdate.h"


namespace SWC { namespace client { namespace Query { namespace Update {



#define SWC_BROKER_COMMIT_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
    SWC_LOG_OSTREAM << " buff-sz=" << cells_buff->fill(); \
  );



BrokerCommitter::BrokerCommitter(
        const Query::Update::Handlers::Base::Ptr& hdlr,
        Query::Update::Handlers::Base::Column* colp) noexcept
        : hdlr(hdlr), colp(colp) {
}

void BrokerCommitter::print(std::ostream& out) {
  out << "BrokerCommitter(cid=" << colp->get_cid()
      << " completion=" << hdlr->completion.count()
      << ')';
}

void BrokerCommitter::commit() {
  hdlr->completion.increment();
  if(!hdlr->valid())
    return hdlr->response();

  workload.increment();
  bool more = true;
  DynamicBuffer::Ptr cells_buff;

  while(more && hdlr->valid() &&
        (cells_buff = colp->get_buff(hdlr->buff_sz, more))) {
    workload.increment();
    Comm::Protocol::Bkr::Req::Committer_CellsUpdate::request(
      shared_from_this(), cells_buff);
  }

  if(workload.is_last())
    hdlr->response();
}

void BrokerCommitter::committed(
                ReqBase::Ptr req,
                const Comm::Protocol::Bkr::Params::CellsUpdateRsp& rsp,
                const DynamicBuffer::Ptr& cells_buff) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit");
      if(workload.is_last())
        hdlr->response();
      return;
    }

    case Error::REQUEST_TIMEOUT: {
      SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
      if(hdlr->valid())
        return req->request_again();
      [[fallthrough]];
    }

    default: {
      hdlr->add_resend_count(colp->add(*cells_buff.get()));
      if(workload.is_last()) {
        SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
        commit();
        hdlr->response();
      }
      return;
    }
  }
}



}}}}
