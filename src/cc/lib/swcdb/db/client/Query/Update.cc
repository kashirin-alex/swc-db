
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Update.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.h"


namespace SWC { namespace client { namespace Query {

namespace Result {

Update::Update() : m_err(Error::OK), m_resend_cells(0) { }

int Update::error() {
  return m_err;
}

void Update::error(int err) {
  m_err.store(err);
}

void Update::add_resend_count(size_t count) {
  m_resend_cells.fetch_add(count);
}

size_t Update::get_resend_count(bool reset) {
  return reset ? m_resend_cells.exchange(0) : m_resend_cells.load();
}

}



Update::Update(const Cb_t& cb, const Comm::IoContextPtr& io)
        : buff_sz(Env::Clients::ref().cfg_send_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_send_ahead->get()), 
          timeout(Env::Clients::ref().cfg_send_timeout->get()), 
          timeout_ratio(Env::Clients::ref().cfg_send_timeout_ratio->get()),
          cb(cb), dispatcher_io(io),
          columns(std::make_shared<DB::Cells::MutableMap>()),
          columns_onfractions(std::make_shared<DB::Cells::MutableMap>()),
          result(std::make_shared<Result>()) { 
}

Update::Update(const DB::Cells::MutableMap::Ptr& columns, 
         const DB::Cells::MutableMap::Ptr& columns_onfractions, 
         const Cb_t& cb, const Comm::IoContextPtr& io)
        : buff_sz(Env::Clients::ref().cfg_send_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_send_ahead->get()), 
          timeout(Env::Clients::ref().cfg_send_timeout->get()), 
          timeout_ratio(Env::Clients::ref().cfg_send_timeout_ratio->get()),
          cb(cb), dispatcher_io(io),
          columns(columns), 
          columns_onfractions(columns_onfractions), 
          result(std::make_shared<Result>()) { 
}

Update::~Update() { }
 
void Update::response(int err) {

  if(!result->completion.is_last()) {
    std::scoped_lock lock(m_mutex);
    return cv.notify_all();
  }

  if(!err && (columns->size() || columns_onfractions->size())) {
    commit();
    std::scoped_lock lock(m_mutex);
    return cv.notify_all();
  }

  if(err)
    result->error(err);
  else if(columns->size() || columns_onfractions->size())
    result->error(Error::CLIENT_DATA_REMAINED);

  result->profile.finished();
  if(cb) {
    dispatcher_io
      ? dispatcher_io->post(
          [updater=shared_from_this()](){ updater->cb(updater->result); })
      : cb(result);
  }

  std::scoped_lock lock(m_mutex);
  cv.notify_all();
}

void Update::wait() {
  std::unique_lock lock_wait(m_mutex);
  cv.wait(
    lock_wait,
    [updater=shared_from_this()]() {
      return !updater->result->completion.count();
    }
  );
}

bool Update::wait_ahead_buffers() {
  std::unique_lock lock_wait(m_mutex);
  
  size_t bytes = columns->size_bytes() + columns_onfractions->size_bytes();
  if(!result->completion.count())
    return bytes >= buff_sz;

  if(bytes < buff_sz * buff_ahead)
    return false;

  cv.wait(
    lock_wait, 
    [updater=shared_from_this()]() {
      return updater->columns->size_bytes() 
              + updater->columns_onfractions->size_bytes()
              < updater->buff_sz * updater->buff_ahead;
    }
  );
  return !result->completion.count() && 
          columns->size_bytes() + columns_onfractions->size_bytes() >= buff_sz;
}


void Update::commit_or_wait(const DB::Cells::ColCells::Ptr& col) {
  if(wait_ahead_buffers())
    col ? commit(col) : commit();
}

void Update::commit_if_need() {
  if(!result->completion.count() && 
     (columns->size_bytes() || columns_onfractions->size_bytes()))
    commit();
}


void Update::commit() {
  result->completion.increment();

  DB::Cells::ColCells::Ptr col;
  for(size_t idx=0; (col=columns->get_idx(idx)); ++idx)
    commit(col);
  for(size_t idx=0; (col=columns_onfractions->get_idx(idx)); ++idx)
    commit_onfractions(col);
  
  response();
}

void Update::commit(const cid_t cid) { 
  commit(columns->get_col(cid));
  commit_onfractions(columns_onfractions->get_col(cid));
}

void Update::commit(const DB::Cells::ColCells::Ptr& col) {
  if(!col || !col->size()) // && !result->cols[cid]->error()
    return;
  result->completion.increment();

  std::make_shared<Locator>(
    DB::Types::Range::MASTER, 
    col->cid, col, col->get_first_key(), 
    shared_from_this()
  )->locate_on_manager();

  response();
}

void Update::commit_onfractions(const DB::Cells::ColCells::Ptr& col) {
  if(col && !col->size())
    return;
  // query cells on fractions -EQ && rest(NONE-true)
  // update cell with + ts,flag,value and add cell to 'columns'
}


#define SWC_LOCATOR_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_LOCATOR_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );

#define SWC_UPDATER_LOCATOR_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    locator->print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );
//

Update::Locator::Locator(const DB::Types::Range type, const cid_t cid, 
        const DB::Cells::ColCells::Ptr& col, 
        const DB::Cell::Key::Ptr& key_start,
        const Update::Ptr& updater, const ReqBase::Ptr& parent, 
        const rid_t rid, const DB::Cell::Key* key_finish) 
        : type(type), cid(cid), col(col), key_start(key_start), 
          updater(updater), parent(parent), 
          rid(rid), 
          key_finish(key_finish ? *key_finish : DB::Cell::Key()) {
}

Update::Locator::~Locator() { }

void Update::Locator::print(std::ostream& out) {
  out << "Locator(type=" << DB::Types::to_string(type)
      << " cid=" << cid
      << " rid=" << rid
      << " completion=" << updater->result->completion.count();
  key_start->print(out << " Start");
  key_finish.print(out << " Finish");
  col->print(out << ' ');
  out << ')';
}

void Update::Locator::locate_on_manager() {
  updater->result->completion.increment();

  Comm::Protocol::Mngr::Params::RgrGetReq params(
    DB::Types::MetaColumn::get_master_cid(col->get_sequence()));
  params.range_begin.copy(*key_start.get());
  if(DB::Types::MetaColumn::is_data(cid))
    params.range_begin.insert(0, std::to_string(cid));
  if(!DB::Types::MetaColumn::is_master(cid))
    params.range_begin.insert(
      0, DB::Types::MetaColumn::get_meta_cid_str(col->get_sequence()));

  SWC_LOCATOR_REQ_DEBUG("mngr_locate_master");

  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile=updater->result->profile.mngr_locate(), 
     locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      locator->located_on_manager(req, rsp);
    }
  );
}


void Update::Locator::located_on_manager(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::COLUMN_NOT_EXISTS: {
      SWC_LOCATOR_RSP_DEBUG("mngr_located_master");
      return updater->response(rsp.err);
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

  auto locator = std::make_shared<Locator>(
    DB::Types::Range::MASTER, 
    rsp.cid, col, key_start, 
    updater, base, 
    rsp.rid
  );
  DB::Types::MetaColumn::is_master(col->cid)
    ? locator->commit_data(rsp.endpoints, base)
    : locator->locate_on_ranger(rsp.endpoints);

  if(!rsp.range_end.empty()) {
    auto next_key_start = col->get_key_next(rsp.range_end);
    if(next_key_start) {
      std::make_shared<Locator>(
        DB::Types::Range::MASTER, 
        col->cid, col, next_key_start, 
        updater
      )->locate_on_manager();
    }
  }
  updater->response();
}

void Update::Locator::locate_on_ranger(const Comm::EndPoints& endpoints) {
  updater->result->completion.increment();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(cid, rid);
  params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::COMMIT;

  params.range_begin.copy(*key_start.get());
  if(!DB::Types::MetaColumn::is_master(col->cid)) {
    params.range_begin.insert(0, std::to_string(col->cid));
    if(type == DB::Types::Range::MASTER && 
       DB::Types::MetaColumn::is_data(col->cid))
      params.range_begin.insert(
        0, DB::Types::MetaColumn::get_meta_cid_str(col->get_sequence()));
  }

  SWC_LOCATOR_REQ_DEBUG("rgr_locate");

  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, endpoints,
    [profile=updater->result->profile.rgr_locate(type),
     locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      locator->located_on_ranger(
        std::dynamic_pointer_cast<
          Comm::Protocol::Rgr::Req::RangeLocate>(req)->endpoints,
        req, rsp
      );
    }
  );
}

void Update::Locator::located_on_ranger(
      const Comm::EndPoints& endpoints, 
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::RANGE_NOT_FOUND:
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::SERVER_SHUTTING_DOWN:
    case Error::COMM_NOT_CONNECTED: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING");
      Env::Clients::get()->rangers.remove(cid, rid);
      return parent->request_again();
    }
    default: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING");
      if(rsp.err != Error::REQUEST_TIMEOUT)
        Env::Clients::get()->rangers.remove(cid, rid);
      return base->request_again();
    }
  }
  if(!rsp.rid) {
    SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING(no-rid)");
    return base->request_again();
  }

  SWC_LOCATOR_RSP_DEBUG("rgr_located");

  std::make_shared<Locator>(
    type == DB::Types::Range::MASTER 
            ? DB::Types::Range::META 
            : DB::Types::Range::DATA,
    rsp.cid, col, key_start, 
    updater, base, 
    rsp.rid, &rsp.range_end
  )->resolve_on_manager();

  if(!rsp.range_end.empty()) {
    auto next_key_start = col->get_key_next(rsp.range_end);
    if(next_key_start) {
      std::make_shared<Locator>(
        type, 
        cid, col, next_key_start, 
        updater, base, 
        rid
      )->locate_on_ranger(endpoints);
    }
  }
  updater->response();
}

void Update::Locator::resolve_on_manager() {
  updater->result->completion.increment();

  Comm::Protocol::Mngr::Params::RgrGetReq params(cid, rid);
  auto req = Comm::Protocol::Mngr::Req::RgrGet::make(
    params,
    [profile=updater->result->profile.mngr_res(), locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      locator->located_ranger(req, rsp);
    }
  );
  if(!DB::Types::MetaColumn::is_master(cid)) {
    Comm::Protocol::Mngr::Params::RgrGetRsp rsp(cid, rid);
    if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
      SWC_LOCATOR_RSP_DEBUG("mngr_resolve_rgr Cache hit");
      return proceed_on_ranger(req, rsp);
    }
  }
  SWC_LOCATOR_REQ_DEBUG("mngr_resolve_rgr");
  req->run();
}

void Update::Locator::located_ranger(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::COLUMN_NOT_EXISTS: {
      SWC_LOCATOR_RSP_DEBUG("rgr_located");
      return updater->response(rsp.err);
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

  if(!DB::Types::MetaColumn::is_master(rsp.cid))
    Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);

  proceed_on_ranger(base, rsp);
}

void Update::Locator::proceed_on_ranger(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(type) {
    case DB::Types::Range::MASTER: {
      if(!DB::Types::MetaColumn::is_master(col->cid))
        goto do_next_locate;
      break;
    }
    case DB::Types::Range::META: {
      if(!DB::Types::MetaColumn::is_meta(col->cid))
        goto do_next_locate;
      break;
    }
    case DB::Types::Range::DATA: {
      break;
    }
  }

  if(cid != rsp.cid || col->cid != cid) {
    Env::Clients::get()->rangers.remove(cid, rid);
    SWC_LOCATOR_RSP_DEBUG("rgr_located RETRYING(cid no match)");
    return (parent ? parent : base)->request_again();;
  }
  commit_data(rsp.endpoints, base);
  return updater->response();

  do_next_locate: {
    std::make_shared<Locator>(
      type,
      rsp.cid, col, key_start,
      updater, base,
      rsp.rid,
      DB::Types::MetaColumn::is_master(cid) ? &rsp.range_end : nullptr
    )->locate_on_ranger(rsp.endpoints);
    return updater->response();
  }
}

void Update::Locator::commit_data(
      const Comm::EndPoints& endpoints,
      const ReqBase::Ptr& base) {
  updater->result->completion.increment();

  bool more = true;
  DynamicBuffer::Ptr cells_buff;
  auto workload = std::make_shared<Core::CompletionCounter<>>(1);

  while(more && 
        (cells_buff = col->get_buff(
          *key_start.get(), key_finish, updater->buff_sz, more))) {
    workload->increment();

    Comm::Protocol::Rgr::Req::RangeQueryUpdate::request(
      Comm::Protocol::Rgr::Params::RangeQueryUpdateReq(col->cid, rid), 
      cells_buff, 
      endpoints, 
      [workload, cells_buff, base, 
       profile=updater->result->profile.rgr_data(), locator=shared_from_this()]
      (ReqBase::Ptr req, 
       const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& rsp) {
        profile.add(rsp.err);
        switch(rsp.err) {

          case Error::OK: {
            SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit");
            if(workload->is_last())
              locator->updater->response();
            return;
          }

          case Error::REQUEST_TIMEOUT: {
            SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
            return req->request_again();
          }

          case Error::RANGE_BAD_INTERVAL:
          case Error::RANGE_BAD_CELLS_INPUT: {
            SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
            size_t resend = locator->col->add(
              *cells_buff.get(), rsp.range_prev_end, rsp.range_end,
              rsp.cells_added, rsp.err == Error::RANGE_BAD_CELLS_INPUT);
            locator->updater->result->add_resend_count(resend);

            if(workload->is_last()) {
              auto nxt_start = locator->col->get_key_next(rsp.range_end);
              if(nxt_start) {
                std::make_shared<Locator>(
                  DB::Types::Range::MASTER,
                  locator->col->cid, locator->col, nxt_start,
                  locator->updater
                )->locate_on_manager();
               }
              locator->updater->response();
            }
            return;
          }

          default: {
            locator->updater->result->add_resend_count(
              locator->col->add(*cells_buff.get())
            );
            if(workload->is_last()) {
              SWC_UPDATER_LOCATOR_RSP_DEBUG("rgr_commit RETRYING");
              base->request_again();
            }
            return;
          }

        }
      },

      updater->timeout + cells_buff->fill()/updater->timeout_ratio
    );

  }

  if(workload->is_last())
    updater->response();
}



}}}
