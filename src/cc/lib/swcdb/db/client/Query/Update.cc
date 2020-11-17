
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

int Update::error() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_err;
}

void Update::error(int err) {
  Core::MutexSptd::scope lock(m_mutex);
  m_err = err;
}

void Update::add_resend_count(size_t count) {
  Core::MutexSptd::scope lock(m_mutex);
  m_resend_cells += count;
}

size_t Update::get_resend_count(bool reset) {
  size_t sz;
  Core::MutexSptd::scope lock(m_mutex);
  sz = m_resend_cells;
  if(reset)
    m_resend_cells = 0;
  return sz;
}

}



Update::Update(const Cb_t& cb, const Comm::IoContext::Ptr& io)
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
         const Cb_t& cb, const Comm::IoContext::Ptr& io)
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
      ? dispatcher_io->post([cb=cb, result=result](){ cb(result); })
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
  DB::Cells::ColCells::Ptr col;
  for(size_t idx=0; (col=columns->get_idx(idx)); ++idx)
    commit(col);
  for(size_t idx=0; (col=columns_onfractions->get_idx(idx)); ++idx)
    commit_onfractions(col);
}

void Update::commit(const cid_t cid) { 
  commit(columns->get_col(cid));
  commit_onfractions(columns_onfractions->get_col(cid));
}

void Update::commit(const DB::Cells::ColCells::Ptr& col) {
  if(col && !col->size())
    return;
  std::make_shared<Locator>(
    DB::Types::Range::MASTER, 
    col->cid, col, col->get_first_key(), 
    shared_from_this()
  )->locate_on_manager();
}

void Update::commit_onfractions(const DB::Cells::ColCells::Ptr& col) {
  if(col && !col->size())
    return;
  // query cells on fractions -EQ && rest(NONE-true)
  // update cell with + ts,flag,value and add cell to 'columns'
}



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
      0, DB::Types::MetaColumn::get_meta_cid(col->get_sequence()));

  SWC_LOG_OUT(LOG_DEBUG, 
    params.print(SWC_LOG_OSTREAM << "LocateRange-onMngr "););

  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile=updater->result->profile.mngr_locate(), 
     locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      if(locator->located_on_manager(req, rsp))
        locator->updater->response();
    }
  );
}


bool Update::Locator::located_on_manager(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  SWC_LOG_OUT(LOG_DEBUG, 
    rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onMngr "););
  
  if(rsp.err == Error::COLUMN_NOT_EXISTS) {
    updater->response(rsp.err);
    return false;
  }
  if(rsp.err || !rsp.rid) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onMngr RETRYING "););
    base->request_again();
    return false;
  }
  
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
  return true;
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
        0, DB::Types::MetaColumn::get_meta_cid(col->get_sequence()));
  }
  
  SWC_LOG_OUT(LOG_DEBUG, 
    params.print(SWC_LOG_OSTREAM << "LocateRange-onRgr "););

  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, endpoints,
    [profile=updater->result->profile.rgr_locate(type),
     locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      if(locator->located_on_ranger(
          std::dynamic_pointer_cast<
            Comm::Protocol::Rgr::Req::RangeLocate>(req)->endpoints,
          req, rsp))
        locator->updater->response();
    }
  );
}

bool Update::Locator::located_on_ranger(
      const Comm::EndPoints& endpoints, 
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  SWC_LOG_OUT(LOG_DEBUG, 
    rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr "););

  if(rsp.err == Error::RGR_NOT_LOADED_RANGE || 
     rsp.err == Error::RANGE_NOT_FOUND || //onMngr can be COLUMN_NOT_EXISTS
         rsp.err == Error::SERVER_SHUTTING_DOWN ||
     rsp.err == Error::COMM_NOT_CONNECTED) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr RETRYING "););
    Env::Clients::get()->rangers.remove(cid, rid);
    parent->request_again();
    return false;
  }
  if(!rsp.rid || rsp.err) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr RETRYING(no rid) "););
    if(rsp.err != Error::REQUEST_TIMEOUT)
      Env::Clients::get()->rangers.remove(cid, rid);
    base->request_again();
    return false;
  }

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
        rid//, &key_finish
      )->locate_on_ranger(endpoints);
    }
  }
  return true;
}

void Update::Locator::resolve_on_manager() {
  updater->result->completion.increment();

  auto req = Comm::Protocol::Mngr::Req::RgrGet::make(
    Comm::Protocol::Mngr::Params::RgrGetReq(cid, rid),
    [profile=updater->result->profile.mngr_res(), locator=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(locator->located_ranger(req, rsp))
        locator->updater->response();
    }
  );
  if(!DB::Types::MetaColumn::is_master(cid)) {
    Comm::Protocol::Mngr::Params::RgrGetRsp rsp(cid, rid);
    if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
      SWC_LOG_OUT(LOG_DEBUG, rsp.print(SWC_LOG_OSTREAM << "Cache hit "););
      if(proceed_on_ranger(req, rsp)) // ?req without profile
        return updater->response();
      Env::Clients::get()->rangers.remove(cid, rid);
    } else {
      SWC_LOG_OUT(LOG_DEBUG, rsp.print(SWC_LOG_OSTREAM << "Cache miss "););
    }
  }
  req->run();
}

bool Update::Locator::located_ranger(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  SWC_LOG_OUT(LOG_DEBUG, 
    rsp.print(SWC_LOG_OSTREAM << "LocatedRanger-onMngr "););

  if(rsp.err) {
    if(rsp.err == Error::COLUMN_NOT_EXISTS) {
      updater->response(rsp.err);
      return false;
    }
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRanger-onMngr RETRYING ");
      print(SWC_LOG_OSTREAM << '\n');
    );
    (parent && rsp.err == Error::RANGE_NOT_FOUND
      ? parent : base)->request_again();
    return false;
  }

  if(!rsp.rid || rsp.endpoints.empty()) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(
        SWC_LOG_OSTREAM << "LocatedRanger-onMngr RETRYING(no rid) "););
    (parent ? parent : base)->request_again();
    return false;
  }

  if(!DB::Types::MetaColumn::is_master(rsp.cid))
    Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);

  return proceed_on_ranger(base, rsp);
}

bool Update::Locator::proceed_on_ranger(
      const ReqBase::Ptr& base, 
      const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(type == DB::Types::Range::DATA || 
     (type == DB::Types::Range::MASTER && 
      DB::Types::MetaColumn::is_master(col->cid)) ||
     (type == DB::Types::Range::META   && 
      DB::Types::MetaColumn::is_meta(col->cid) )) {
    if(cid != rsp.cid || col->cid != cid) {
      SWC_LOG_OUT(LOG_DEBUG, 
        rsp.print(
          SWC_LOG_OSTREAM << "LocatedRanger-onMngr RETRYING(cid no match) "););
      (parent ? parent : base)->request_again();
      //updater->response(Error::NOT_ALLOWED);
      return false;
    }
    commit_data(rsp.endpoints, base);

  } else {
    std::make_shared<Locator>(
      type, 
      rsp.cid, col, key_start, 
      updater, base, 
      rsp.rid, 
      DB::Types::MetaColumn::is_master(cid) ? &rsp.range_end : nullptr
    )->locate_on_ranger(rsp.endpoints);
  }
  return true;
}

void Update::Locator::commit_data(
      const Comm::EndPoints& endpoints,
      const ReqBase::Ptr& base) {
  bool more = true;
  DynamicBuffer::Ptr cells_buff;
  auto workload = std::make_shared<Core::CompletionCounter<>>();
  while(more && 
       (cells_buff = col->get_buff(
         *key_start.get(), key_finish, updater->buff_sz, more))) {
    workload->increment();
    updater->result->completion.increment();

    Comm::Protocol::Rgr::Req::RangeQueryUpdate::request(
      Comm::Protocol::Rgr::Params::RangeQueryUpdateReq(col->cid, rid), 
      cells_buff, 
      endpoints, 
      [workload, cells_buff, base, 
       profile=updater->result->profile.rgr_data(), locator=shared_from_this()]
      (ReqBase::Ptr req, 
       const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& rsp) {
        profile.add(rsp.err);

        if(rsp.err) {
          SWC_LOG_OUT(LOG_DEBUG, 
            rsp.print(SWC_LOG_OSTREAM << "Commit RETRYING ");
            SWC_LOG_OSTREAM << " buffs=" << workload->count();
          );

          switch(rsp.err) {

            case Error::REQUEST_TIMEOUT: {
              SWC_LOG_OUT(LOG_DEBUG,  req->print(SWC_LOG_OSTREAM ); );
              return req->request_again();
            }

            case Error::RANGE_BAD_INTERVAL:
            case Error::RANGE_BAD_CELLS_INPUT: {
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
              }
              break;
            }

            default: {
              locator->updater->result->add_resend_count(
                locator->col->add(*cells_buff.get())
              );
              if(workload->is_last())
                return base->request_again();
            }
          }
        }

        locator->updater->response();
      },

      updater->timeout + cells_buff->fill()/updater->timeout_ratio
    );

  }
}



}}}
