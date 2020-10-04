
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"


namespace SWC { namespace client { namespace Query {
 
namespace Result {


Select::Rsp::Rsp() : m_err(Error::OK) {
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

size_t Select::Rsp::get_size() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size();
}

size_t Select::Rsp::get_size_bytes() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size_bytes();
}

bool Select::Rsp::empty() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.empty();
}

void Select::Rsp::free() {
  Core::MutexSptd::scope lock(m_mutex);
  m_cells.free();
}


void Select::Rsp::error(int err) {
  Core::MutexSptd::scope lock(m_mutex);
  m_err = err;
}

int Select::Rsp::error() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_err;
}


Select::Select(bool notify) 
              : notify(notify), err(Error::OK),  
                m_completion(0) {
}

Select::~Select() { }

uint32_t Select::completion() {
  std::scoped_lock lock(mutex);
  return m_completion;
}

uint32_t Select::_completion() const {
  return m_completion;
}

void Select::completion_incr() {
  std::scoped_lock lock(mutex);
  ++m_completion;
}

void Select::completion_decr() {
  std::scoped_lock lock(mutex);
  --m_completion;
}

bool Select::completion_final() {
  std::scoped_lock lock(mutex);
  return !--m_completion;
}

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

size_t Select::get_size_bytes() {
  size_t sz = 0;
  for(const auto& col : m_columns)
    sz += col.second->get_size_bytes();
  return sz;
}

bool Select::empty() const {
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



Select::Select(const Cb_t& cb, bool rsp_partials)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()), 
          timeout(Env::Clients::ref().cfg_recv_timeout->get()), 
          cb(cb), 
          result(std::make_shared<Result>(cb && rsp_partials)),
          m_rsp_partial_runs(false) { 
}

Select::Select(const DB::Specs::Scan& specs, const Cb_t& cb, 
               bool rsp_partials)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()), 
          timeout(Env::Clients::ref().cfg_recv_timeout->get()), 
          cb(cb), specs(specs),
          result(std::make_shared<Result>(cb && rsp_partials)),
          m_rsp_partial_runs(false) {
}

Select::~Select() { }
 
void Select::response(int err) {
  if(err)
    result->err = err;
  
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
    cb(result);
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
    
  cb(result);

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
             !selector->result->_completion();
    }
  );
  if(result->notify && !result->empty())
    cb(result);
}

void Select::scan(int& err) {
  std::vector<DB::Types::KeySeq> sequences;
  DB::Schema::Ptr schema;
  for(auto& col : specs.columns) {
    schema = Env::Clients::get()->schemas->get(err, col->cid);
    if(err)
      return;
    result->add_column(col->cid);
    sequences.push_back(schema->col_seq);
  }

  auto it_seq = sequences.begin();
  for(auto& col : specs.columns) {
    for(auto& intval : col->intervals) {
      if(!intval->flags.max_buffer)
        intval->flags.max_buffer = buff_sz;
      std::make_shared<ScannerColumn>(
        col->cid, *it_seq, *intval.get(), 
        shared_from_this()
      )->run();
    }
    ++it_seq;
  }
}


Select::ScannerColumn::ScannerColumn(const cid_t cid, 
                                     const DB::Types::KeySeq col_seq,
                                     DB::Specs::Interval& interval,
                                     const Select::Ptr& selector)
                                    : cid(cid), col_seq(col_seq), 
                                      interval(interval), selector(selector) {
}

Select::ScannerColumn::~ScannerColumn() { }

void Select::ScannerColumn::run() {
  std::make_shared<Scanner>(
    DB::Types::Range::MASTER, cid, shared_from_this()
  )->locate_on_manager(false);
}

bool Select::ScannerColumn::add_cells(const StaticBuffer& buffer, 
                                      bool reached_limit) {
  return selector->result->add_cells(cid, buffer, reached_limit, interval);
}

void Select::ScannerColumn::next_call(bool final) {
  if(next_calls.empty()) {
    if(final && !selector->result->completion())
      selector->response(Error::OK);
    return;
  }
  interval.offset_key.free();
  interval.offset_rev = 0;

  auto call = next_calls.back();
  next_calls.pop_back();
  call();
}

void Select::ScannerColumn::add_call(const std::function<void()>& call) {    
  next_calls.push_back(call);
}

void Select::ScannerColumn::print(std::ostream& out) {
  out << "ScannerColumn("
      << "cid=" << cid;
  interval.print(out << ' ');
  out << " completion=" << selector->result->completion()
      << " next_calls=" << next_calls.size()
      << ')';
}


Select::Scanner::Scanner(
        const DB::Types::Range type, const cid_t cid, 
        const ScannerColumn::Ptr& col, const ReqBase::Ptr& parent, 
        const DB::Cell::Key* range_offset, const rid_t rid)
      : type(type), cid(cid), col(col), parent(parent), rid(rid),
        range_offset(range_offset ? *range_offset : DB::Cell::Key()) {
}

Select::Scanner::~Scanner() {}

void Select::Scanner::print(std::ostream& out) {
  out << "Scanner(type=" << DB::Types::to_string(type)
      << " cid=" << cid
      << " rid=" << rid;
  range_offset.print(out << " RangeOffset");
  col->print(out << ' ');
  out << ' ';
}
    
void Select::Scanner::locate_on_manager(bool next_range) {
  col->selector->result->completion_incr();

  Comm::Protocol::Mngr::Params::RgrGetReq params(
    DB::Types::MetaColumn::get_master_cid(col->col_seq), 0, next_range);

  if(!range_offset.empty()) {
    params.range_begin.copy(range_offset);
    col->interval.apply_possible_range_end(params.range_end); 
  } else {
    col->interval.apply_possible_range(
      params.range_begin, params.range_end);
  }
  
  if(DB::Types::MetaColumn::is_data(cid)) {
    params.range_begin.insert(0, std::to_string(cid));
    params.range_end.insert(0, std::to_string(cid));
  }
  if(!DB::Types::MetaColumn::is_master(cid)) {
    auto meta_cid = DB::Types::MetaColumn::get_meta_cid(col->col_seq);
    params.range_begin.insert(0, meta_cid);
    params.range_end.insert(0, meta_cid);
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    params.print(SWC_LOG_OSTREAM << "LocateRange-onMngr "););

  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [next_range, profile=col->selector->result->profile.mngr_locate(),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      if(scanner->located_on_manager(req, rsp, next_range))
        scanner->col->selector->result->completion_decr();
    }
  );
}

void Select::Scanner::resolve_on_manager() {
  col->selector->result->completion_incr();

  auto req = Comm::Protocol::Mngr::Req::RgrGet::make(
    Comm::Protocol::Mngr::Params::RgrGetReq(cid, rid),
    [profile=col->selector->result->profile.mngr_res(), 
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->located_on_manager(req, rsp))
        scanner->col->selector->result->completion_decr();
    }
  );
  if(!DB::Types::MetaColumn::is_master(cid)) {
    Comm::Protocol::Mngr::Params::RgrGetRsp rsp(cid, rid);
    if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
      SWC_LOG_OUT(LOG_DEBUG, rsp.print(SWC_LOG_OSTREAM << "Cache hit "); );
      if(proceed_on_ranger(req, rsp)) // ?req without profile
        return col->selector->result->completion_decr(); 
      Env::Clients::get()->rangers.remove(cid, rid);
    } else {
      SWC_LOG_OUT(LOG_DEBUG, rsp.print(SWC_LOG_OSTREAM << "Cache miss "); );
    }
  }
  req->run();
}

bool Select::Scanner::located_on_manager(
        const ReqBase::Ptr& base, 
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp, 
        bool next_range) {
  SWC_LOG_OUT(LOG_DEBUG, 
    rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onMngr "););

  if(rsp.err) {
    if(rsp.err == Error::COLUMN_NOT_EXISTS) {
      col->selector->result->err = rsp.err; // Error::CONSIST_ERRORS;
      col->selector->result->error(col->cid, rsp.err);
      if(col->selector->result->completion_final())
        col->selector->response();
      return false;
    }
    if(next_range && rsp.err == Error::RANGE_NOT_FOUND) {
      col->selector->result->completion_decr();
      col->next_call(true);
      return false;
    }

    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "Located-onMngr RETRYING "););
    if(rsp.err == Error::RANGE_NOT_FOUND) {
      (parent == nullptr ? base : parent)->request_again();
    } else {
      base->request_again();
    }
    return false;
  }
  if(!rsp.rid) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "Located-onMngr RETRYING(no rid) "););
    (parent == nullptr ? base : parent)->request_again();
    return false;
  }

  if(type == DB::Types::Range::MASTER) {
    col->add_call(
      [scanner=std::make_shared<Scanner>(
        type, cid, col,
        parent == nullptr ? base : parent, &rsp.range_begin
      )] () { scanner->locate_on_manager(true); }
    );
  } else if(!DB::Types::MetaColumn::is_master(rsp.cid)) {
    Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);
  }

  return proceed_on_ranger(base, rsp);
}
    
bool Select::Scanner::proceed_on_ranger(
          const ReqBase::Ptr& base, 
          const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(type == DB::Types::Range::DATA || 
     (type == DB::Types::Range::MASTER && 
      DB::Types::MetaColumn::is_master(col->cid)) ||
     (type == DB::Types::Range::META   && 
      DB::Types::MetaColumn::is_meta(col->cid))) {

    if(cid != rsp.cid || col->cid != cid) {
      SWC_LOG_OUT(LOG_DEBUG, 
        rsp.print(
          SWC_LOG_OSTREAM << "Located-onMngr RETRYING(cid no match) "););
      (parent == nullptr ? base : parent)->request_again();
      return false;
      //col->selector->response(Error::NOT_ALLOWED);
      //return true;
    }
    select(rsp.endpoints, rsp.rid, base);

  } else {
    std::make_shared<Scanner>(
      type, rsp.cid, col,
      base, nullptr, rsp.rid
    )->locate_on_ranger(rsp.endpoints, false);
  }
  return true;
}

void Select::Scanner::locate_on_ranger(const Comm::EndPoints& endpoints, 
                                       bool next_range) {
  col->selector->result->completion_incr();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(cid, rid);
  if(next_range) {
    params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(range_offset);
    params.range_offset.insert(0, std::to_string(col->cid));
  }

  col->interval.apply_possible_range(params.range_begin, params.range_end);
  params.range_begin.insert(0, std::to_string(col->cid));
  params.range_end.insert(0, std::to_string(col->cid));
  if(type == DB::Types::Range::MASTER && 
     DB::Types::MetaColumn::is_data(col->cid)) {
    auto meta_cid = DB::Types::MetaColumn::get_meta_cid(col->col_seq);
    params.range_begin.insert(0, meta_cid);
    params.range_end.insert(0, meta_cid);
    if(next_range)
      params.range_offset.insert(0, meta_cid);
  }

  SWC_LOG_OUT(LOG_DEBUG, 
    params.print(SWC_LOG_OSTREAM << "LocateRange-onRgr "););

  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, endpoints,
    [next_range, profile=col->selector->result->profile.rgr_locate(type),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      if(scanner->located_on_ranger(
          std::dynamic_pointer_cast<
            Comm::Protocol::Rgr::Req::RangeLocate>(req)->endpoints,
          req, rsp, next_range))
        scanner->col->selector->result->completion_decr();
    }
  );
}

bool Select::Scanner::located_on_ranger(
          const Comm::EndPoints& endpoints, 
          const ReqBase::Ptr& base, 
          const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp, 
          bool next_range) {
  SWC_LOG_OUT(LOG_DEBUG, 
    rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr "););

  if(rsp.err) {
    if(rsp.err == Error::RANGE_NOT_FOUND && 
       (next_range || type == DB::Types::Range::META)) {
      col->selector->result->completion_decr();
      col->next_call(true);
      return false;
    }

    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr RETRYING "););                
    if(rsp.err == Error::RGR_NOT_LOADED_RANGE || 
       rsp.err == Error::RANGE_NOT_FOUND  || 
       rsp.err == Error::SERVER_SHUTTING_DOWN ||
       rsp.err == Error::COMM_NOT_CONNECTED) {
        Env::Clients::get()->rangers.remove(cid, rid);
      parent->request_again();
    } else {
      base->request_again();
    }
    return false;
  }
  if(!rsp.rid) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(SWC_LOG_OSTREAM << "LocatedRange-onRgr RETRYING(no rid) "););
    Env::Clients::get()->rangers.remove(cid, rid);
    parent->request_again();
    return false;
  }
  if(type == DB::Types::Range::DATA && rsp.cid != col->cid) {
    SWC_LOG_OUT(LOG_DEBUG, 
      rsp.print(
        SWC_LOG_OSTREAM << "LocatedRange-onRgr RETRYING(cid no match "););    
    Env::Clients::get()->rangers.remove(cid, rid);
    parent->request_again();
    return false;
  }

  if(type != DB::Types::Range::DATA) {
    col->add_call([endpoints, scanner=std::make_shared<Scanner>(
      type, cid, col,
      parent, &rsp.range_begin, rid
      )] () { scanner->locate_on_ranger(endpoints, true); }
    );
  }

  std::make_shared<Scanner>(
    type == DB::Types::Range::MASTER 
            ? DB::Types::Range::META 
            : DB::Types::Range::DATA,
    rsp.cid, col, 
    parent, nullptr, rsp.rid
  )->resolve_on_manager();

  return true;
}

void Select::Scanner::select(const Comm::EndPoints& endpoints, 
                             rid_t rid, 
                             const ReqBase::Ptr& base) {
  col->selector->result->completion_incr();

  Comm::Protocol::Rgr::Req::RangeQuerySelect::request(
    Comm::Protocol::Rgr::Params::RangeQuerySelectReq(
      col->cid, rid, col->interval
    ), 
    endpoints, 
    [rid, base, 
     profile=col->selector->result->profile.rgr_data(), 
     scanner=shared_from_this()] 
    (const ReqBase::Ptr& req, 
     const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
      profile.add(rsp.err);
      
      if(rsp.err) {
        SWC_LOG_OUT(LOG_DEBUG, 
          rsp.print(SWC_LOG_OSTREAM << "Select RETRYING "); );

        if(rsp.err == Error::RGR_NOT_LOADED_RANGE || 
           rsp.err == Error::SERVER_SHUTTING_DOWN ||
           rsp.err == Error::COMM_NOT_CONNECTED) {
          Env::Clients::get()->rangers.remove(scanner->col->cid, rid);
          base->request_again();
        } else {
          req->request_again();
        }
        return;
      }
      auto& col = scanner->col;

      if(col->interval.flags.offset)
        col->interval.flags.offset = rsp.offset;

      if(!rsp.data.size || col->add_cells(rsp.data, rsp.reached_limit)) {
        if(rsp.data.size)
          col->selector->response_partials();

        if(rsp.reached_limit) {
          scanner->select(
            std::dynamic_pointer_cast<
              Comm::Protocol::Rgr::Req::RangeQuerySelect>(req)->endpoints,
            rid, base
          );
        } else {
          col->next_call();
        }
      }

          
      if(col->selector->result->completion_final())
        col->selector->response();
    },

    col->selector->timeout
  );
}

  
  
}}}
