/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/Types/SystemColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.h"


namespace SWC { namespace client { namespace Query { namespace Update {



#define SWC_LOCATOR_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_LOCATOR_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );



Committer::Committer(const DB::Types::Range a_type,
                     const cid_t a_cid,
                     Handlers::Base::Column* a_colp,
                     const DB::Cell::Key::Ptr& a_key_start,
                     const Handlers::Base::Ptr& a_hdlr,
                     const ReqBase::Ptr& a_parent,
                     const rid_t a_rid) noexcept
  : type(a_type),
    master_cid(DB::Types::SystemColumn::get_master_cid(a_colp->get_sequence())),
    workload(0),
    cid(a_cid), colp(a_colp),
    key_start(a_key_start),
    hdlr(a_hdlr), parent(a_parent), rid(a_rid) {
}

Committer::Committer(const DB::Types::Range a_type,
                     const cid_t a_cid,
                     Handlers::Base::Column* a_colp,
                     const DB::Cell::Key::Ptr& a_key_start,
                     const Handlers::Base::Ptr& a_hdlr,
                     const ReqBase::Ptr& a_parent,
                     const rid_t a_rid,
                     const DB::Cell::Key& a_key_finish)
  : type(a_type),
    master_cid(DB::Types::SystemColumn::get_master_cid(a_colp->get_sequence())),
    workload(0),
    cid(a_cid), colp(a_colp),
    key_start(a_key_start),
    hdlr(a_hdlr), parent(a_parent), rid(a_rid), key_finish(a_key_finish) {
}

Committer::~Committer() noexcept { }

void Committer::print(std::ostream& out) {
  out << "Committer(type=" << DB::Types::to_string(type)
      << " cid=" << cid << " rid=" << rid
      << " completion=" << hdlr->completion.count();
  key_start->print(out << " Start");
  key_finish.print(out << " Finish");
  colp->print(out << ' ');
  out << ')';
}


struct ReqDataBase {
  Committer::Ptr committer;
  SWC_CAN_INLINE
  ReqDataBase(const Committer::Ptr& a_committer) noexcept
              : committer(a_committer) { }
  SWC_CAN_INLINE
  ~ReqDataBase() noexcept { }
  SWC_CAN_INLINE
  client::Clients::Ptr& get_clients() noexcept {
    return committer->hdlr->clients;
  }
  SWC_CAN_INLINE
  bool valid() {
    return committer->hdlr->valid();
  }
};


void Committer::locate_on_manager() {
  hdlr->completion.increment();
  if(!hdlr->valid())
    return hdlr->response();

  Profiling::Component::Start profile(hdlr->profile.mngr_locate());

  Comm::Protocol::Mngr::Params::RgrGetReq params(master_cid);
  params.range_begin.copy(*key_start.get());
  if(DB::Types::SystemColumn::is_data(cid))
    params.range_begin.insert(0, std::to_string(cid));
  if(!DB::Types::SystemColumn::is_master(cid))
    params.range_begin.insert(
      0, DB::Types::SystemColumn::get_meta_cid_str(colp->get_sequence()));

  SWC_LOCATOR_REQ_DEBUG("mngr_locate_master");
  struct ReqData : ReqDataBase {
    Profiling::Component::Start profile;
    SWC_CAN_INLINE
    ReqData(const Ptr& a_committer,
            Profiling::Component::Start& a_profile) noexcept
            : ReqDataBase(a_committer), profile(a_profile) { }
    SWC_CAN_INLINE
    ~ReqData() noexcept { }
    SWC_CAN_INLINE
    void callback(const ReqBase::Ptr& req,
                  Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      committer->located_on_manager(req, rsp);
    };
  };

  auto req = Comm::Protocol::Mngr::Req::RgrGet<ReqData>
    ::make(params, 10000, shared_from_this(), profile);

  if(!DB::Types::SystemColumn::is_master(colp->get_cid())) {
    rid_t             master_rid;
    DB::Cell::Key     key_end;
    Comm::EndPoints   master_rgr_endpoints;
    int64_t           master_mngr_revision;

    if(hdlr->clients->mngr_cache_get_write_master(
        master_cid, params.range_begin,
        master_rid, key_end, master_rgr_endpoints, master_mngr_revision)) {
      profile.add_cached(Error::OK);
      SWC_LOG_OUT(LOG_DEBUG,
        SWC_LOG_OSTREAM
          << "mngr_located_master Cache hit revision=" << master_mngr_revision
          << " Ranger(cid=" << master_cid << " rid=" << master_rid
          << " endpoints=[";
          for(auto& endpoint : master_rgr_endpoints)
            SWC_LOG_OSTREAM << endpoint << ',';
          SWC_LOG_OSTREAM << "])";
      );
      Ptr(new Committer(
        DB::Types::Range::MASTER,
        master_cid, colp, key_start,
        hdlr, req,
        master_rid
      ))->locate_on_ranger(
        std::move(master_rgr_endpoints), master_mngr_revision);

      if(key_end.count > 2) {
        key_end.remove(0); // master-cid
        key_end.remove(0); // meta-cid
        auto next_key_start = colp->get_key_next(key_end);
        if(next_key_start) {
          Ptr(new Committer(
            DB::Types::Range::MASTER,
            colp->get_cid(), colp, next_key_start,
            hdlr
          ))->locate_on_manager();
        }
      }
      return hdlr->response();
    }
  }

  req->run();
}


void Committer::located_on_manager(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(!hdlr->valid())
    return hdlr->response();
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::COLUMN_NOT_EXISTS: {
      SWC_LOCATOR_RSP_DEBUG("mngr_located_master");
      if(cid == colp->get_cid())
        hdlr->error(cid, rsp.err);
      return hdlr->response(rsp.err);
    }
    case Error::CLIENT_STOPPING: {
      SWC_LOCATOR_RSP_DEBUG("mngr_located_master STOPPED");
      hdlr->error(rsp.err);
      return hdlr->response(rsp.err);
    }
    default: {
      SWC_LOCATOR_RSP_DEBUG("mngr_located_master RETRYING");
      return base->request_again();
    }
  }
  if(!rsp.rid) {
    SWC_LOCATOR_RSP_DEBUG("mngr_located_master RETRYING(no-rid)");
    return base->request_again();
  }

  SWC_LOCATOR_RSP_DEBUG("mngr_located_master");

  hdlr->clients->mngr_cache_set_master(
    master_cid, rsp.rid,
    rsp.range_begin, rsp.range_end,
    rsp.endpoints, rsp.revision
  );

  Ptr committer(new Committer(
    DB::Types::Range::MASTER,
    master_cid, colp, key_start,
    hdlr, base,
    rsp.rid
  ));
  DB::Types::SystemColumn::is_master(colp->get_cid())
    ? committer->commit_data(std::move(rsp.endpoints), base)
    : committer->locate_on_ranger(std::move(rsp.endpoints), rsp.revision);

  if(rsp.range_end.count > 2) {
    DB::Cell::Key key(rsp.range_end);
    key.remove(0); // master-cid
    key.remove(0); // meta-cid
    auto next_key_start = colp->get_key_next(key);
    if(next_key_start) {
      Ptr(new Committer(
        DB::Types::Range::MASTER,
        colp->get_cid(), colp, next_key_start,
        hdlr
      ))->locate_on_manager();
    }
  }
  hdlr->response();
}

void Committer::locate_on_ranger(Comm::EndPoints&& endpoints,
                                 int64_t revision) {
  hdlr->completion.increment();
  if(!hdlr->valid())
    return hdlr->response();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(cid, rid);
  params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::COMMIT;
  if(revision) {
    params.revision = revision;
    params.flags |=
      Comm::Protocol::Rgr::Params::RangeLocateReq::HAVE_REVISION;
  }

  params.range_begin.copy(*key_start.get());
  if(!DB::Types::SystemColumn::is_master(colp->get_cid())) {
    params.range_begin.insert(0, std::to_string(colp->get_cid()));
    if(type == DB::Types::Range::MASTER &&
       DB::Types::SystemColumn::is_data(colp->get_cid()))
      params.range_begin.insert(
        0, DB::Types::SystemColumn::get_meta_cid_str(colp->get_sequence()));
  }

  SWC_LOCATOR_REQ_DEBUG("locate_on_ranger");
  struct ReqData : ReqDataBase {
    Profiling::Component::Start profile;
    const Comm::EndPoints       endpoints;
    SWC_CAN_INLINE
    ReqData(const Ptr& a_committer, Comm::EndPoints& a_endpoints,
            DB::Types::Range type) noexcept
            : ReqDataBase(a_committer),
              profile(committer->hdlr->profile.rgr_locate(type)),
              endpoints(std::move(a_endpoints)) { }
    SWC_CAN_INLINE
    ~ReqData() noexcept { }
    SWC_CAN_INLINE
    void callback(const ReqBase::Ptr& req,
                  const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      committer->located_on_ranger(endpoints, req, rsp);
    }
  };

  auto req = Comm::Protocol::Rgr::Req::RangeLocate<ReqData>
    ::make(params, 10000, shared_from_this(), std::move(endpoints), type);
  req->request(req, req->data.endpoints);
}

void Committer::located_on_ranger(
      const Comm::EndPoints& endpoints,
      const ReqBase::Ptr& base,
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  if(!hdlr->valid())
    return hdlr->response();
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::RANGE_NOT_FOUND:
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::SERVER_SHUTTING_DOWN:
    case Error::COMM_NOT_CONNECTED: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING");
      cid == master_cid
        ? hdlr->clients->mngr_cache_remove_master(cid, rid)
        : hdlr->clients->rgr_cache_remove(cid, rid);
      return parent->request_again();
    }
    default: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING");
      if(rsp.err != Error::REQUEST_TIMEOUT) {
        cid == master_cid
          ? hdlr->clients->mngr_cache_remove_master(cid, rid)
          : hdlr->clients->rgr_cache_remove(cid, rid);
      }
      return base->request_again();
    }
  }
  if(!rsp.rid) {
    SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING(no-rid)");
    return base->request_again();
  }

  SWC_LOCATOR_RSP_DEBUG("rgr_located");

  Ptr(new Committer(
    type == DB::Types::Range::MASTER
            ? DB::Types::Range::META
            : DB::Types::Range::DATA,
    rsp.cid, colp, key_start,
    hdlr, base,
    rsp.rid, rsp.range_end
  ))->resolve_on_manager();

  if(!rsp.range_end.empty()) {
    auto next_key_start = colp->get_key_next(rsp.range_end);
    if(next_key_start) {
      Ptr(new Committer(
        type,
        cid, colp, next_key_start,
        hdlr, base,
        rid
      ))->locate_on_ranger(Comm::EndPoints(endpoints));
    }
  }
  hdlr->response();
}

void Committer::resolve_on_manager() {
  hdlr->completion.increment();
  if(!hdlr->valid())
    return hdlr->response();

  struct ReqData : ReqDataBase {
    Profiling::Component::Start profile;
    SWC_CAN_INLINE
    ReqData(const Ptr& a_committer) noexcept
            : ReqDataBase(a_committer),
              profile(committer->hdlr->profile.mngr_res()) { }
    SWC_CAN_INLINE
    ~ReqData() noexcept { }
    SWC_CAN_INLINE
    void callback(const ReqBase::Ptr& req,
                  Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      committer->located_ranger(req, rsp);
    };
  };

  Comm::Protocol::Mngr::Params::RgrGetReq params(cid, rid);
  auto req = Comm::Protocol::Mngr::Req::RgrGet<ReqData>
    ::make(params, 10000, shared_from_this());

  if(!DB::Types::SystemColumn::is_master(cid)) {
    auto profile = hdlr->profile.mngr_res();
    Comm::Protocol::Mngr::Params::RgrGetRsp rsp(cid, rid);
    if(hdlr->clients->rgr_cache_get(cid, rid, rsp.endpoints)) {
      profile.add_cached(Error::OK);
      SWC_LOCATOR_RSP_DEBUG("mngr_resolve_rgr Cache hit");
      return proceed_on_ranger(req, rsp);
    }
  }
  SWC_LOCATOR_REQ_DEBUG("mngr_resolve_rgr");
  req->run();
}

void Committer::located_ranger(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(!hdlr->valid())
    return hdlr->response();
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::COLUMN_NOT_EXISTS: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located");
      if(cid == colp->get_cid()) {
        hdlr->clients->schemas.remove(cid);
        hdlr->error(cid, rsp.err);
      }
      return hdlr->response(rsp.err);
    }
    case Error::CLIENT_STOPPING: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located STOPPED");
      hdlr->error(rsp.err);
      return hdlr->response(rsp.err);
    }
    default: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING");
      return (parent && rsp.err == Error::RANGE_NOT_FOUND
              ? parent : base)->request_again();
    }
  }
  if(!rsp.rid || rsp.endpoints.empty()) {
    SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING(no-rid)");
    return (parent ? parent : base)->request_again();
  }

  SWC_LOCATOR_RSP_DEBUG("rgr_located");

  if(!DB::Types::SystemColumn::is_master(rsp.cid))
    hdlr->clients->rgr_cache_set(rsp.cid, rsp.rid, rsp.endpoints);

  proceed_on_ranger(base, rsp);
}

void Committer::proceed_on_ranger(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(!hdlr->valid())
    return hdlr->response();
  switch(type) {
    case DB::Types::Range::MASTER: {
      if(!DB::Types::SystemColumn::is_master(colp->get_cid()))
        goto do_next_locate;
      break;
    }
    case DB::Types::Range::META: {
      if(!DB::Types::SystemColumn::is_meta(colp->get_cid()))
        goto do_next_locate;
      break;
    }
    case DB::Types::Range::DATA: {
      break;
    }
  }

  if(cid != rsp.cid || colp->get_cid() != cid) {
    hdlr->clients->rgr_cache_remove(cid, rid);
    SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING(cid no match)");
    return (parent ? parent : base)->request_again();
  }
  commit_data(rsp.endpoints, base);
  return hdlr->response();

  do_next_locate: {
     Ptr(
      DB::Types::SystemColumn::is_master(cid)
      ? new Committer(
          type,
          rsp.cid, colp, key_start,
          hdlr, base,
          rsp.rid, rsp.range_end)
      : new Committer(
          type,
          rsp.cid, colp, key_start,
          hdlr, base,
          rsp.rid)
    )->locate_on_ranger(std::move(rsp.endpoints));
    return hdlr->response();
  }
}

void Committer::commit_data(
      const Comm::EndPoints& endpoints,
      const ReqBase::Ptr& base) {
  hdlr->completion.increment();

  struct ReqData : ReqDataBase {
    Profiling::Component::Start  profile;
    DynamicBuffer                cells_buff;
    ReqBase::Ptr                 base;
    SWC_CAN_INLINE
    ReqData(const Ptr& a_committer,
            DynamicBuffer& a_cells_buff,
            const ReqBase::Ptr& a_base) noexcept
            : ReqDataBase(a_committer),
              profile(committer->hdlr->profile.rgr_data()),
              cells_buff(std::move(a_cells_buff)), base(a_base) {
    }
    SWC_CAN_INLINE
    ~ReqData() noexcept { }
    SWC_CAN_INLINE
    void callback(
            const ReqBase::Ptr& req,
            const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& rsp) {
      profile.add(rsp.err);
      committer->committed_data(cells_buff, base, req, rsp);
    }
  };

  bool more = true;
  DynamicBuffer cells_buff;
  workload.increment();

  while(more && hdlr->valid() &&
        colp->get_buff(
          *key_start.get(), key_finish, hdlr->buff_sz, more, cells_buff)) {
    workload.increment();

    Comm::Protocol::Rgr::Req::RangeQueryUpdate<ReqData>
      ::request(
          Comm::Protocol::Rgr::Params::RangeQueryUpdateReq(
            colp->get_cid(), rid),
          cells_buff,
          hdlr->timeout + cells_buff.fill()/hdlr->timeout_ratio,
          endpoints,
          shared_from_this(),
          std::move(cells_buff),
          base
        );
  }
  if(workload.is_last())
    hdlr->response();
}


#define SWC_UPDATER_LOCATOR_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );

void Committer::committed_data(
        const DynamicBuffer& cells_buff,
        const ReqBase::Ptr& base,
        const ReqBase::Ptr& req,
        const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit");
      if(workload.is_last())
        hdlr->response();
      return;
    }

    case Error::REQUEST_TIMEOUT: {
      SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
      return req->request_again();
    }

    case Error::RANGE_BAD_INTERVAL:
    case Error::RANGE_BAD_CELLS_INPUT: {
      SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
      size_t resend = colp->add(
        cells_buff, rsp.range_prev_end, rsp.range_end,
        rsp.cells_added, rsp.err == Error::RANGE_BAD_CELLS_INPUT
      );
      hdlr->add_resend_count(resend);

      if(workload.is_last()) {
        auto nxt_start = colp->get_key_next(rsp.range_end);
        if(nxt_start) {
          Ptr(new Committer(
            DB::Types::Range::MASTER,
            colp->get_cid(), colp, nxt_start,
            hdlr
          ))->locate_on_manager();
        }
        hdlr->response();
      }
      return;
    }

    default: {
      hdlr->add_resend_count(colp->add(cells_buff));
      if(workload.is_last()) {
        SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
        base->request_again();
      }
      return;
    }
  }
}

#undef SWC_UPDATER_LOCATOR_RSP_DEBUG

}}}}
