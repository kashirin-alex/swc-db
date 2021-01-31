
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select.h"


namespace SWC { namespace client { namespace Query {

namespace Result {


Select::Rsp::Rsp() noexcept : m_err(Error::OK) {
}

Select::Rsp::~Rsp() { }

bool Select::Rsp::add_cells(const StaticBuffer& buffer, bool reached_limit,
                            DB::Specs::Interval& interval) {
  Core::MutexSptd::scope lock(m_mutex);
  size_t recved = m_cells.add(buffer.base, buffer.size);

  if(interval.flags.limit) {
    if(interval.flags.limit <= recved) {
      interval.flags.limit = 0;
      return false;
    }
    interval.flags.limit -= recved;
  }

  if(reached_limit) {
    auto last = m_cells.back();
    interval.offset_key.copy(last->key);
    interval.offset_rev = last->get_timestamp();
  }
  return true;
}

void Select::Rsp::get_cells(DB::Cells::Result& cells) {
  Core::MutexSptd::scope lock(m_mutex);
  cells.take(m_cells);
}

size_t Select::Rsp::get_size() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size();
}

size_t Select::Rsp::get_size_bytes() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size_bytes();
}

bool Select::Rsp::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.empty();
}

void Select::Rsp::free() {
  Core::MutexSptd::scope lock(m_mutex);
  m_cells.free();
}


void Select::Rsp::error(int err) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  m_err = err;
}

int Select::Rsp::error() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_err;
}


Select::Select(bool notify) noexcept
              : notify(notify), err(Error::OK),
                completion(0) {
}

Select::~Select() { }

void Select::error(const cid_t cid, int err) {
  m_columns[cid]->error(err);
}

void Select::add_column(const cid_t cid) {
  m_columns.emplace(cid, std::make_shared<Rsp>());
}

Select::Rsp::Ptr Select::get_columnn(const cid_t cid) {
  return m_columns[cid];
}

bool Select::add_cells(const cid_t cid, const StaticBuffer& buffer,
                       bool reached_limit, DB::Specs::Interval& interval) {
  return m_columns[cid]->add_cells(buffer, reached_limit, interval);
}

void Select::get_cells(const cid_t cid, DB::Cells::Result& cells) {
  m_columns[cid]->get_cells(cells);
  if(notify) {
    std::scoped_lock lock(mutex);
    cv.notify_all();
  }
}

size_t Select::get_size(const cid_t cid) {
  return m_columns[cid]->get_size();
}

size_t Select::get_size_bytes() const noexcept {
  size_t sz = 0;
  for(const auto& col : m_columns)
    sz += col.second->get_size_bytes();
  return sz;
}

bool Select::empty() const noexcept {
  for(const auto& col : m_columns)
    if(!col.second->empty())
      return false;
  return true;
}

std::vector<cid_t> Select::get_cids() const {
  std::vector<cid_t> list(m_columns.size());
  int i = 0;
  for(const auto& col : m_columns)
    list[i++] = col.first;
  return list;
}

void Select::free(const cid_t cid) {
  m_columns[cid]->free();
  if(notify) {
    std::scoped_lock lock(mutex);
    cv.notify_all();
  }
}

void Select::remove(const cid_t cid) {
  // unused
  m_columns.erase(cid);
}

} // namespace Result



Select::Select(const Cb_t& cb, bool rsp_partials,
               const Comm::IoContextPtr& io)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()),
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()),
          timeout(Env::Clients::ref().cfg_recv_timeout->get()),
          cb(cb), dispatcher_io(io),
          result(std::make_shared<Result>(cb && rsp_partials)),
          m_rsp_partial_runs(false) {
}

Select::Select(const DB::Specs::Scan& specs,
               const Cb_t& cb, bool rsp_partials,
               const Comm::IoContextPtr& io)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()),
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()),
          timeout(Env::Clients::ref().cfg_recv_timeout->get()),
          cb(cb), dispatcher_io(io), specs(specs),
          result(std::make_shared<Result>(cb && rsp_partials)),
          m_rsp_partial_runs(false) {
}

Select::~Select() { }

void Select::response(int err) {
  if(err)
    result->err.store(err);

  result->profile.finished();

  if(result->notify) {
    bool call;
    {
      std::scoped_lock lock(result->mutex);
      if((call = !m_rsp_partial_runs))
        m_rsp_partial_runs = true;
    }
    if(call)
      response_partial();
  } else if(cb) {
    send_result();
  }

  std::scoped_lock lock(result->mutex);
  result->cv.notify_all();
}

void Select::response_partials() {
  if(!result->notify)
    return;

  {
    std::unique_lock lock_wait(result->mutex);
    if(m_rsp_partial_runs) {
      if(wait_on_partials()) {
        result->cv.wait(
          lock_wait,
          [selector=shared_from_this()] () {
            return !selector->m_rsp_partial_runs ||
                   !selector->wait_on_partials();
          }
        );
      }
      return;
    }
    m_rsp_partial_runs = true;
  }
  Env::IoCtx::post(
    [selector=shared_from_this()] () { selector->response_partial(); });
}

bool Select::wait_on_partials() const {
  return result->get_size_bytes() > buff_sz * buff_ahead;
}

void Select::response_partial() {

  send_result();

  std::scoped_lock lock(result->mutex);
  m_rsp_partial_runs = false;
  result->cv.notify_all();
}

void Select::wait() {
  std::unique_lock lock_wait(result->mutex);
  result->cv.wait(
    lock_wait,
    [selector=shared_from_this()] () {
      return !selector->m_rsp_partial_runs &&
             !selector->result->completion.count();
    }
  );
  if(result->notify && !result->empty())
    send_result();
}

void Select::send_result() {
  dispatcher_io
    ? dispatcher_io->post(
        [selector=shared_from_this()](){ selector->cb(selector->result); })
    : cb(result);
}

void Select::scan(int& err) {
  std::vector<DB::Schema::Ptr> schemas(specs.columns.size());
  auto it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    auto schema = Env::Clients::get()->schemas->get(err, col->cid);
    if(err)
      return;
    result->add_column(col->cid);
    *it_seq++ = schema;
    result->completion.increment();
  }

  it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    for(auto& intval : col->intervals) {
      if(!intval->flags.max_buffer)
        intval->flags.max_buffer = buff_sz;
      if(!intval->values.empty())
        intval->values.col_type = (*it_seq)->col_type;
      std::make_shared<Scanner>(
        shared_from_this(),
        (*it_seq)->col_seq, *intval.get(), col->cid
      )->mngr_locate_master();
    }
    ++it_seq;
  }
}

#define SWC_SCANNER_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_SCANNER_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );


Select::Scanner::Scanner(const Select::Ptr& selector,
                         const DB::Types::KeySeq col_seq,
                         DB::Specs::Interval& interval,
                         const cid_t cid)
            : completion(0),
              selector(selector),
              col_seq(col_seq),
              interval(interval),
              master_cid(DB::Types::MetaColumn::get_master_cid(col_seq)),
              meta_cid(DB::Types::MetaColumn::get_meta_cid(col_seq)),
              data_cid(cid),
              master_rid(0),
              meta_rid(0),
              data_rid(0),
              master_mngr_next(false),
              master_rgr_next(false),
              meta_next(false) {
}

Select::Scanner::~Scanner() { }

void Select::Scanner::print(std::ostream& out) {
  out << "Scanner(" << DB::Types::to_string(col_seq)
      << " master(" << master_cid << '/' << master_rid
        << " mngr-next=" << master_mngr_next
        << " rgr-next=" << master_rgr_next << ')'
      << " meta(" << meta_cid << '/' << meta_rid
        << " next=" << meta_next << ')'
      << " data(" << data_cid << '/' << data_rid  << ") ";
  interval.print(out);
  out << " completion=" << completion.count() << ')';
}

bool Select::Scanner::add_cells(const StaticBuffer& buffer,
                                bool reached_limit) {
  return selector->result->add_cells(data_cid, buffer, reached_limit, interval);
}

void Select::Scanner::response_if_last() {
  if(completion.is_last()) {
    master_rgr_req_base = nullptr;
    meta_req_base = nullptr;
    data_req_base = nullptr;
    if(selector->result->completion.is_last())
      selector->response(Error::OK);
  }
}

void Select::Scanner::next_call() {
  if(meta_next) {
    if(!interval.offset_key.empty() && !meta_end.empty()) {
      if(DB::KeySeq::compare(col_seq, interval.offset_key, meta_end)
         != Condition::LT) {
        interval.offset_key.free();
        interval.offset_rev = 0;
      }
    }
    rgr_locate_meta();

  } else if(master_rgr_next) {
    rgr_locate_master();

  } else if(master_mngr_next) {
    mngr_locate_master();
  }
}


//
void Select::Scanner::mngr_locate_master() {
  completion.increment();

  Comm::Protocol::Mngr::Params::RgrGetReq params(
    master_cid, 0, master_mngr_next);

  if(master_mngr_next) {
    params.range_begin.copy(master_mngr_offset);
  }
  /*** MASTER columns need to have aligned MetaData interval
  if(master_mngr_offset.empty()) {
    interval.apply_possible_range(
      params.range_begin, params.range_end);
  } else {
    params.range_begin.copy(master_mngr_offset);
    interval.apply_possible_range_end(params.range_end);
  }
  ***/

  if(DB::Types::MetaColumn::is_data(data_cid)) {
    auto data_cid_str = std::to_string(data_cid);
    params.range_begin.insert(0, data_cid_str);
    params.range_end.insert(0, data_cid_str);
  }
  if(!DB::Types::MetaColumn::is_master(data_cid)) {
    auto meta_cid_str = DB::Types::MetaColumn::get_meta_cid_str(col_seq);
    params.range_begin.insert(0, meta_cid_str);
    params.range_end.insert(0, meta_cid_str);
  }

  SWC_SCANNER_REQ_DEBUG("mngr_locate_master");
  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile=selector->result->profile.mngr_locate(),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      if(scanner->mngr_located_master(req, rsp))
        scanner->response_if_last();
    }
  );
}

bool Select::Scanner::mngr_located_master(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {

  if(rsp.err) {
    if(master_mngr_next && rsp.err == Error::RANGE_NOT_FOUND) {
      master_mngr_next = false;
      SWC_SCANNER_RSP_DEBUG("mngr_located_master finished");
      return true;
    }
    SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING");
    req->request_again();
    return false;
  }
  if(!rsp.rid) {
    SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING(no rid)");
    req->request_again();
    return false;
  }
  if(master_cid != rsp.cid) {
    SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING(cid no match)");
    req->request_again();
    return false;
  }

  SWC_SCANNER_RSP_DEBUG("mngr_located_master");
  Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);
  master_mngr_next = true;
  master_mngr_offset.copy(rsp.range_begin);
  if(DB::Types::MetaColumn::is_master(data_cid)) {
    data_rid = rsp.rid;
    data_req_base = req;
    data_endpoints = rsp.endpoints;
    rgr_select();
  } else {
    master_rid = rsp.rid;
    master_rgr_req_base = req;
    master_rgr_endpoints = rsp.endpoints;
    rgr_locate_master();
  }
  return true;
}


//
void Select::Scanner::rgr_locate_master() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(master_cid, master_rid);
  auto data_cid_str = std::to_string(data_cid);
  if(master_rgr_next) {
    params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(master_rgr_offset);
    params.range_offset.insert(0, data_cid_str);
  }

  /*** MASTER columns need to have aligned MetaData interval
  bool range_end_rest = false;
  interval.apply_possible_range(
    params.range_begin, params.range_end, &range_end_rest);
  if(range_end_rest)
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST;
  if(interval.has_opt__key_equal())
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::KEY_EQUAL;
  ***/

  params.range_begin.insert(0, data_cid_str);
  params.range_end.insert(0, data_cid_str);
  if(DB::Types::MetaColumn::is_data(data_cid)) {
    auto meta_cid_str = DB::Types::MetaColumn::get_meta_cid_str(col_seq);
    params.range_begin.insert(0, meta_cid_str);
    params.range_end.insert(0, meta_cid_str);
    if(master_rgr_next)
      params.range_offset.insert(0, meta_cid_str);
  }

  SWC_SCANNER_REQ_DEBUG("rgr_locate_master");
  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, master_rgr_endpoints,
    [profile=selector->result->profile.rgr_locate(DB::Types::Range::MASTER),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      scanner->rgr_located_master(req, rsp);
    }
  );
}

void Select::Scanner::rgr_located_master(
          const ReqBase::Ptr& req,
          const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  if(rsp.err) {
    if(master_rgr_next && rsp.err == Error::RANGE_NOT_FOUND) {
      master_rgr_next = false;
      SWC_SCANNER_RSP_DEBUG("rgr_located_master master_rgr_next");
      next_call();
      return response_if_last();
    }
    SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING");
    if(rsp.err == Error::RGR_NOT_LOADED_RANGE ||
       rsp.err == Error::RANGE_NOT_FOUND  ||
       rsp.err == Error::SERVER_SHUTTING_DOWN ||
       rsp.err == Error::COMM_NOT_CONNECTED) {
      Env::Clients::get()->rangers.remove(master_cid, master_rid);
      return master_rgr_req_base->request_again();
    } else {
      return req->request_again();
    }
  }
  if(!rsp.rid) {
    SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING(no rid)");
    Env::Clients::get()->rangers.remove(master_cid, master_rid);
    return master_rgr_req_base->request_again();
  }
  if(meta_cid != rsp.cid) {
    SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING(cid no match)");
    Env::Clients::get()->rangers.remove(master_cid, master_rid);
    return master_rgr_req_base->request_again();
  }

  SWC_SCANNER_RSP_DEBUG("rgr_located_master");
  master_rgr_next = true;
  master_rgr_offset.copy(rsp.range_begin);
  if(DB::Types::MetaColumn::is_meta(data_cid)) {
    data_rid = rsp.rid;
    data_req_base = req;
    mngr_resolve_rgr_select();
  } else {
    meta_rid = rsp.rid;
    meta_req_base = req;
    mngr_resolve_rgr_meta();
  }
  response_if_last();
}


//
void Select::Scanner::mngr_resolve_rgr_meta() {
  completion.increment();

  auto profile = selector->result->profile.mngr_res();
  auto params = Comm::Protocol::Mngr::Params::RgrGetReq(meta_cid, meta_rid);
  auto req = Comm::Protocol::Mngr::Req::RgrGet::make(
    params,
    [profile, scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->mngr_resolved_rgr_meta(req, rsp))
        scanner->response_if_last();
    }
  );

  Comm::Protocol::Mngr::Params::RgrGetRsp rsp(meta_cid, meta_rid);
  if(Env::Clients::get()->rangers.get(meta_cid, meta_rid, rsp.endpoints)) {
    SWC_SCANNER_RSP_DEBUG("mngr_resolve_rgr_meta Cache hit");
    if(mngr_resolved_rgr_meta(req, rsp)) {
      profile.add(Error::OK);
      return response_if_last();
    }
    Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
  }
  SWC_SCANNER_REQ_DEBUG("mngr_resolve_rgr_meta");
  req->run();
}

bool Select::Scanner::mngr_resolved_rgr_meta(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(rsp.err) {
    SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_meta RETRYING");
    (rsp.err == Error::RANGE_NOT_FOUND? meta_req_base : req)->request_again();
    return false;
  }
  SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_meta");
  meta_endpoints = rsp.endpoints;
  Env::Clients::get()->rangers.set(meta_cid, meta_rid, rsp.endpoints);
  rgr_locate_meta();
  return true;
}


//
void Select::Scanner::rgr_locate_meta() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(meta_cid, meta_rid);
  auto data_cid_str = std::to_string(data_cid);
  if(meta_next) {
    params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(meta_offset);
    params.range_offset.insert(0, data_cid_str);
  }

  bool range_end_rest = false;
  interval.apply_possible_range(
    params.range_begin, params.range_end, &range_end_rest);
  if(range_end_rest ||
     (!interval.range_end.empty() && interval.has_opt__range_end_rest()))
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST;
  if(interval.has_opt__key_equal())
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::KEY_EQUAL;

  params.range_begin.insert(0, data_cid_str);
  params.range_end.insert(0, data_cid_str);

  SWC_SCANNER_REQ_DEBUG("rgr_locate_meta");
  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, meta_endpoints,
    [profile=selector->result->profile.rgr_locate(DB::Types::Range::META),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      scanner->rgr_located_meta(req, rsp);
    }
  );
}

void Select::Scanner::rgr_located_meta(
          const ReqBase::Ptr& req,
          const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  switch(rsp.err) {
    case Error::OK:
      break;
    case Error::RANGE_NOT_FOUND: {
      meta_next = false;
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta meta_next");
      next_call();
      return response_if_last();
    }
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::COMM_NOT_CONNECTED:
    case Error::SERVER_SHUTTING_DOWN: {
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING");
      Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
      return meta_req_base->request_again();
    }
    default: {
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING-locate");
      return req->request_again();
    }
  }
  if(!rsp.rid) {
    SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING(no rid)");
    Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
    return meta_req_base->request_again();
  }
  if(data_cid != rsp.cid) {
    SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING(cid no match)");
    Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
    return meta_req_base->request_again();
  }

  SWC_SCANNER_RSP_DEBUG("rgr_located_meta)");
  meta_next = true;
  meta_offset.copy(rsp.range_begin);
  meta_end.copy(rsp.range_end);
  data_rid = rsp.rid;
  data_req_base = req;
  mngr_resolve_rgr_select();
  response_if_last();
}


//
void Select::Scanner::mngr_resolve_rgr_select() {
  completion.increment();

  auto profile = selector->result->profile.mngr_res();
  Comm::Protocol::Mngr::Params::RgrGetReq params(data_cid, data_rid);
  auto req = Comm::Protocol::Mngr::Req::RgrGet::make(
    params,
    [profile, scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->mngr_resolved_rgr_select(req, rsp))
        scanner->response_if_last();
    }
  );

  Comm::Protocol::Mngr::Params::RgrGetRsp rsp(data_cid, data_rid);
  if(Env::Clients::get()->rangers.get(data_cid, data_rid, rsp.endpoints)) {
    SWC_SCANNER_RSP_DEBUG("mngr_resolve_rgr_select Cache hit");
    if(mngr_resolved_rgr_select(req, rsp)) {
      profile.add(Error::OK);
      return response_if_last();
    }
    Env::Clients::get()->rangers.remove(data_cid, data_rid);
  }
  SWC_SCANNER_REQ_DEBUG("mngr_resolve_rgr_select");
  req->run();
}

bool Select::Scanner::mngr_resolved_rgr_select(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(rsp.err) {
    if(rsp.err == Error::COLUMN_NOT_EXISTS) {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select QUIT");
      selector->result->err.store(rsp.err); // Error::CONSIST_ERRORS;
      selector->result->error(data_cid, rsp.err);
      return true;
    }
    SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select RETRYING");
    (rsp.err == Error::RANGE_NOT_FOUND? data_req_base : req)->request_again();
    return false;
  }

  SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select");
  data_endpoints = rsp.endpoints;
  Env::Clients::get()->rangers.set(data_cid, data_rid, rsp.endpoints);
  rgr_select();
  return true;
}


//
void Select::Scanner::rgr_select() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeQuerySelectReq params(
    data_cid, data_rid, interval);

  SWC_SCANNER_REQ_DEBUG("rgr_select");
  Comm::Protocol::Rgr::Req::RangeQuerySelect::request(
    params, data_endpoints,
    [profile=selector->result->profile.rgr_data(), scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
      profile.add(rsp.err);
      scanner->rgr_selected(req, rsp);
    },
    selector->timeout
  );
}

void Select::Scanner::rgr_selected(
                const ReqBase::Ptr& req,
                const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
  if(rsp.err) {
    SWC_SCANNER_RSP_DEBUG("rgr_selected RETRYING");
    if(rsp.err == Error::RGR_NOT_LOADED_RANGE ||
       rsp.err == Error::SERVER_SHUTTING_DOWN ||
       rsp.err == Error::COMM_NOT_CONNECTED) {
      Env::Clients::get()->rangers.remove(data_cid, data_rid);
      return data_req_base->request_again();
    } else {
      return req->request_again();
    }
  }

  SWC_SCANNER_RSP_DEBUG("rgr_selected");
  if(interval.flags.offset)
    interval.flags.offset = rsp.offset;

  if(!rsp.data.size || add_cells(rsp.data, rsp.reached_limit)) {
    if(rsp.data.size)
      selector->response_partials();
    rsp.reached_limit ? rgr_select() : next_call();
  }
  response_if_last();
}


#undef SWC_SCANNER_REQ_DEBUG
#undef SWC_SCANNER_RSP_DEBUG

}}}
