/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/BrokerCommitter.h"
#include "swcdb/db/Protocol/Bkr/req/CellsUpdate.h"

namespace SWC { namespace client { namespace Query { namespace Update {



#define SWC_BROKER_COMMIT_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
    SWC_LOG_OSTREAM << " buff-sz=" << cells_buff.fill(); \
  );



void BrokerCommitter::print(std::ostream& out) {
  out << "BrokerCommitter(cid=" << colp->get_cid()
      << " completion=" << hdlr->completion.count()
      << ')';
}

void BrokerCommitter::commit() {
  hdlr->completion.increment();
  if(!hdlr->valid())
    return hdlr->response();

  struct ReqData {
    Ptr                          committer;
    Profiling::Component::Start  profile;
    DynamicBuffer                cells_buff;
    SWC_CAN_INLINE
    ReqData(const Ptr& committer, DynamicBuffer& cells_buff) noexcept
            : committer(committer),
              profile(committer->hdlr->profile.bkr()),
              cells_buff(std::move(cells_buff)) {
    }
    SWC_CAN_INLINE
    client::Clients::Ptr& get_clients() noexcept {
      return committer->hdlr->clients;
    }
    SWC_CAN_INLINE
    bool valid() {
      return committer->hdlr->valid();
    }
    SWC_CAN_INLINE
    void callback(
            const ReqBase::Ptr& req,
            const Comm::Protocol::Bkr::Params::CellsUpdateRsp& rsp) {
      profile.add(rsp.err);
      committer->committed(req, rsp, cells_buff);
    }
  };

  workload.increment();
  bool more = true;
  DynamicBuffer cells_buff;

  while(more && hdlr->valid() &&
        colp->get_buff(hdlr->buff_sz, more, cells_buff)) {
    workload.increment();
    Comm::Protocol::Bkr::Req::CellsUpdate<ReqData>
      ::request(
          Comm::Protocol::Bkr::Params::CellsUpdateReq(colp->get_cid()),
          cells_buff,
          hdlr->timeout + cells_buff.fill()/hdlr->timeout_ratio,
          shared_from_this(),
          std::move(cells_buff)
        );
  }

  if(workload.is_last())
    hdlr->response();
}

void BrokerCommitter::committed(
                ReqBase::Ptr req,
                const Comm::Protocol::Bkr::Params::CellsUpdateRsp& rsp,
                const DynamicBuffer& cells_buff) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit");
      if(workload.is_last())
        hdlr->response();
      return;
    }

    case Error::COLUMN_NOT_EXISTS: {
      SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit");
      hdlr->clients->schemas.remove(colp->get_cid());
      hdlr->add_resend_count(colp->add(cells_buff));
      hdlr->error(colp->get_cid(), rsp.err);
      if(workload.is_last())
        hdlr->response(rsp.err);
      return;
    }

    case Error::CLIENT_STOPPING: {
      SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit STOPPED");
      hdlr->add_resend_count(colp->add(cells_buff));
      hdlr->error(rsp.err);
      if(workload.is_last())
        hdlr->response(rsp.err);
      return;
    }

    case Error::COMM_NOT_CONNECTED:
    case Error::REQUEST_TIMEOUT: {
      if(hdlr->valid()) {
        SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
        return req->request_again();
      }
      [[fallthrough]];
    }

    default: {
      hdlr->add_resend_count(colp->add(cells_buff));
      if(workload.is_last()) {
        if(hdlr->valid()) {
          SWC_BROKER_COMMIT_RSP_DEBUG("bkr_commit RETRYING");
          commit();
        }
        hdlr->response();
      }
      return;
    }
  }
}



}}}}
