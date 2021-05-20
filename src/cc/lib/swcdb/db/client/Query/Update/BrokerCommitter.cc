/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/BrokerCommitter.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Bkr/req/CellsUpdate.h"


namespace SWC { namespace client { namespace Query { namespace Update {



#define SWC_BROKER_COMMIT_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    committer->print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );



BrokerCommitter::BrokerCommitter(
        const Query::Update::Handlers::Base::Ptr& hdlr,
        Query::Update::Handlers::Base::Column* colp) noexcept
        : hdlr(hdlr), colp(colp) {
}

void BrokerCommitter::print(std::ostream& out) {
  out << "BrokerCommitter(completion=" << hdlr->completion.count()
      << " endpoints=[";
  for(auto& e : endpoints)
    out << e << ',';
  out << "])";
}

void BrokerCommitter::run() {
  hdlr->completion.increment();
  while((endpoints = hdlr->clients->brokers.get_endpoints()).empty()) {
    SWC_LOG(LOG_ERROR, "Broker hosts cfg 'swc.bkr.host' is empty, waiting!");
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  if(hdlr->valid())
    commit_data();
  hdlr->response();
}

void BrokerCommitter::commit_data() {
  hdlr->completion.increment();

  bool more = true;
  DynamicBuffer::Ptr cells_buff;
  auto workload = std::make_shared<Core::CompletionCounter<>>(1);

  while(more && hdlr->valid() &&
        (cells_buff = colp->get_buff(hdlr->buff_sz, more))) {
    workload->increment();

    Comm::Protocol::Bkr::Req::CellsUpdate::request(
      hdlr->clients, endpoints, colp->get_cid(), cells_buff,
      [workload, cells_buff,
       profile=hdlr->profile.rgr_data(), committer=shared_from_this()]
      (ReqBase::Ptr req,
       const Comm::Protocol::Bkr::Params::CellsUpdateRsp& rsp) {
        profile.add(rsp.err);
        switch(rsp.err) {

          case Error::OK: {
            SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit");
            if(workload->is_last())
              committer->hdlr->response();
            return;
          }

          case Error::REQUEST_TIMEOUT: {
            SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
            if(committer->hdlr->valid())
              return req->request_again();
            [[fallthrough]];
          }

          default: {
            committer->hdlr->add_resend_count(
              committer->colp->add(*cells_buff.get())
            );
            if(workload->is_last()) {
              SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
              committer->run();
              committer->hdlr->response();
            }
            return;
          }

        }
      },

      hdlr->timeout + cells_buff->fill()/hdlr->timeout_ratio
    );

  }

  if(workload->is_last())
    hdlr->response();
}



}}}}
