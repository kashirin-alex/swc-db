
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select.h"


namespace SWC { namespace client { namespace Query {
 
namespace Result {


Select::Rsp::Rsp() { }

Select::Rsp::~Rsp() { }

bool Select::Rsp::add_cells(const StaticBuffer& buffer, bool reached_limit, 
                            DB::Specs::Interval& interval) {
  Mutex::scope lock(m_mutex);
  size_t recved = m_cells.add(buffer.base, buffer.size);
  m_counted += recved;
  m_size_bytes += buffer.size;

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
  Mutex::scope lock(m_mutex);
  cells.take(m_cells);
  m_size_bytes = 0;
}

size_t Select::Rsp::get_size() {
  Mutex::scope lock(m_mutex);
  return m_counted;
}

size_t Select::Rsp::get_size_bytes() {
  Mutex::scope lock(m_mutex);
  return m_size_bytes;
}

void Select::Rsp::free() {
  Mutex::scope lock(m_mutex);
  m_cells.free();
  m_size_bytes = 0;
}


Select::Select(std::condition_variable& cv, bool notify) 
              : notify(notify), m_cv(cv) { 
}

Select::~Select() { }

void Select::add_column(const int64_t cid) {
  m_columns.emplace(cid, std::make_shared<Rsp>());
}
  
bool Select::add_cells(const int64_t cid, const StaticBuffer& buffer, 
                       bool reached_limit, DB::Specs::Interval& interval) {
  return m_columns[cid]->add_cells(buffer, reached_limit, interval);
}

void Select::get_cells(const int64_t cid, DB::Cells::Result& cells) {
  m_columns[cid]->get_cells(cells);
  if(notify)
    m_cv.notify_all();
}

size_t Select::get_size(const int64_t cid) {
  return m_columns[cid]->get_size();
}

size_t Select::get_size_bytes() {
  size_t sz = 0;
  for(const auto& col : m_columns)
    sz += col.second->get_size_bytes();
  return sz;
}

std::vector<int64_t> Select::get_cids() const {
  std::vector<int64_t> list(m_columns.size());
  int i = 0;
  for(const auto& col : m_columns)
    list[i++] = col.first;
  return list;
}

void Select::free(const int64_t cid) {
  m_columns[cid]->free();
  if(notify)
    m_cv.notify_all();
}

void Select::remove(const int64_t cid) {
  m_columns.erase(cid);
}
  
} // namespace Result



Select::Select(Cb_t cb, bool rsp_partials)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()), 
          timeout(Env::Clients::ref().cfg_recv_timeout->get()), 
          cb(cb), rsp_partials(cb && rsp_partials), 
          result(std::make_shared<Result>(m_cv, rsp_partials)) { 
}

Select::Select(const DB::Specs::Scan& specs, Cb_t cb, bool rsp_partials)
        : buff_sz(Env::Clients::ref().cfg_recv_buff_sz->get()), 
          buff_ahead(Env::Clients::ref().cfg_recv_ahead->get()), 
          timeout(Env::Clients::ref().cfg_recv_timeout->get()), 
          cb(cb), specs(specs), rsp_partials(cb && rsp_partials),
          result(std::make_shared<Result>(m_cv, rsp_partials)) {
}

Select::~Select() { 
  result->notify.store(false);
}
 
void Select::response(int err) {
  if(err)
    result->err = err;
  // if cells not empty commit-again
  
  result->profile.finished();
  if(cb)
    cb(result);

  m_cv.notify_all();
}

void Select::response_partials() {
  if(!rsp_partials)
    return;
    
  response_partial();
    
  if(!wait_on_partials())
    return;

  std::unique_lock lock(m_mutex);
  if(!m_rsp_partial_runs)
    return;
  m_cv.wait(
    lock, 
    [selector=shared_from_this()] () { 
      return !selector->m_rsp_partial_runs || !selector->wait_on_partials();
    }
  );  
}

bool Select::wait_on_partials() const {
  return result->get_size_bytes() > buff_sz * buff_ahead;
}

void Select::response_partial() {
  {
    std::unique_lock lock(m_mutex);
    if(m_rsp_partial_runs)
      return;
    m_rsp_partial_runs = true;
  }
    
  cb(result);

  {
    std::unique_lock lock(m_mutex);
    m_rsp_partial_runs = false;
  }
  m_cv.notify_all();
}

void Select::wait() {
  std::unique_lock lock(m_mutex);
  m_cv.wait(
    lock, 
    [selector=shared_from_this()] () {
      return selector->result->completion == 0 && 
            !selector->m_rsp_partial_runs;
    }
  );
}

void Select::scan(int& err) {
  std::vector<Types::KeySeq> sequences;
  DB::Schema::Ptr schema;
  for(auto &col : specs.columns) {
    schema = Env::Clients::get()->schemas->get(err, col->cid);
    if(err)
      return;
    result->add_column(col->cid);
    sequences.push_back(schema->col_seq);
  }

  auto it_seq = sequences.begin();
  for(auto &col : specs.columns) {
    for(auto &intval : col->intervals) {
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


Select::ScannerColumn::ScannerColumn(const int64_t cid, const Types::KeySeq col_seq,
                                     DB::Specs::Interval& interval,
                                     const Select::Ptr& selector)
                                    : cid(cid), col_seq(col_seq), 
                                      interval(interval), selector(selector) {
}

Select::ScannerColumn::~ScannerColumn() { }

void Select::ScannerColumn::run() {
  std::make_shared<Scanner>(
    Types::Range::MASTER, cid, shared_from_this()
  )->locate_on_manager(false);
}

bool Select::ScannerColumn::add_cells(const StaticBuffer& buffer, 
                                      bool reached_limit) {
  return selector->result->add_cells(cid, buffer, reached_limit, interval);
}

void Select::ScannerColumn::next_call(bool final) {
  if(next_calls.empty()) {
    if(final && !selector->result->completion)
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

std::string Select::ScannerColumn::to_string() {
  std::string s("ScannerColumn(");
  s.append("cid=");
  s.append(std::to_string(cid));
  s.append(" ");
  s.append(interval.to_string());
  s.append(" completion=");
  s.append(std::to_string(selector->result->completion.load()));
  s.append(" next_calls=");
  s.append(std::to_string(next_calls.size()));
  s.append(")");
  return s;
}


Select::Scanner::Scanner(
        const Types::Range type, const int64_t cid, 
        const ScannerColumn::Ptr& col, const ReqBase::Ptr& parent, 
        const DB::Cell::Key* range_offset, const int64_t rid)
      : type(type), cid(cid), col(col), parent(parent), 
        range_offset(range_offset ? *range_offset : DB::Cell::Key()), 
        rid(rid) {
}

Select::Scanner::~Scanner() {}

std::string Select::Scanner::to_string() {
  std::string s("Scanner(type=");
  s.append(Types::to_string(type));
  s.append(" cid=");
  s.append(std::to_string(cid));
  s.append(" rid=");
  s.append(std::to_string(rid));
  s.append(" RangeOffset");
  s.append(range_offset.to_string());
  s.append(" ");
  s.append(col->to_string());
  s.append(")");
  return s;
}
    
void Select::Scanner::locate_on_manager(bool next_range) {
  ++col->selector->result->completion;

  Protocol::Mngr::Params::RgrGetReq params(
    Types::MetaColumn::get_master_cid(col->col_seq), 0, next_range);

  if(!range_offset.empty()) {
    params.range_begin.copy(range_offset);
    col->interval.apply_possible_range_end(params.range_end); 
  } else {
    col->interval.apply_possible_range(
      params.range_begin, params.range_end);
  }
  
  if(Types::MetaColumn::is_data(cid)) {
    params.range_begin.insert(0, std::to_string(cid));
    params.range_end.insert(0, std::to_string(cid));
  }
  if(!Types::MetaColumn::is_master(cid)) {
    auto meta_cid = Types::MetaColumn::get_meta_cid(col->col_seq);
    params.range_begin.insert(0, meta_cid);
    params.range_end.insert(0, meta_cid);
  }

  SWC_LOGF(LOG_DEBUG, "LocateRange-onMngr %s", params.to_string().c_str());

  Protocol::Mngr::Req::RgrGet::request(
    params,
    [next_range, profile=col->selector->result->profile.mngr_locate(),
     scanner=shared_from_this()]
    (ReqBase::Ptr req, const Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      if(scanner->located_on_manager(req, rsp, next_range))
        --scanner->col->selector->result->completion;
    }
  );
}

void Select::Scanner::resolve_on_manager() {

  auto req = Protocol::Mngr::Req::RgrGet::make(
    Protocol::Mngr::Params::RgrGetReq(cid, rid),
    [profile=col->selector->result->profile.mngr_res(), 
     scanner=shared_from_this()]
    (ReqBase::Ptr req, const Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->located_on_manager(req, rsp))
        --scanner->col->selector->result->completion;
    }
  );
  if(!Types::MetaColumn::is_master(cid)) {
    Protocol::Mngr::Params::RgrGetRsp rsp(cid, rid);
    if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
      SWC_LOGF(LOG_DEBUG, "Cache hit %s", rsp.to_string().c_str());
      if(proceed_on_ranger(req, rsp)) // ?req without profile
        return; 
      Env::Clients::get()->rangers.remove(cid, rid);
    } else {
      SWC_LOGF(LOG_DEBUG, "Cache miss %s", rsp.to_string().c_str());
    }
  }
  ++col->selector->result->completion;
  req->run();
}

bool Select::Scanner::located_on_manager(
        const ReqBase::Ptr& base, 
        const Protocol::Mngr::Params::RgrGetRsp& rsp, 
        bool next_range) {
  SWC_LOGF(LOG_DEBUG, "LocatedRange-onMngr %s", rsp.to_string().c_str());

  if(rsp.err) {
    if(rsp.err == Error::COLUMN_NOT_EXISTS) {
      col->selector->response(rsp.err);
      return true;
    }
    if(next_range && rsp.err == Error::RANGE_NOT_FOUND) {
      --col->selector->result->completion;
      col->next_call(true);
      return false;
    }

    SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING %s", 
                          rsp.to_string().c_str());
    if(rsp.err == Error::RANGE_NOT_FOUND) {
      (parent == nullptr ? base : parent)->request_again();
    } else {
      base->request_again();
    }
    return false;
  }
  if(!rsp.rid) {
    SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING(no rid) %s", 
                        rsp.to_string().c_str());
    (parent == nullptr ? base : parent)->request_again();
    return false;
  }

  if(type == Types::Range::MASTER) {
    col->add_call(
      [scanner=std::make_shared<Scanner>(
        type, cid, col,
        parent == nullptr ? base : parent, &rsp.range_begin
      )] () { scanner->locate_on_manager(true); }
    );
  } else if(!Types::MetaColumn::is_master(rsp.cid)) {
    Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);
  }

  return proceed_on_ranger(base, rsp);
}
    
bool Select::Scanner::proceed_on_ranger(
          const ReqBase::Ptr& base, 
          const Protocol::Mngr::Params::RgrGetRsp& rsp) {
  if(type == Types::Range::DATA || 
    (type == Types::Range::MASTER && Types::MetaColumn::is_master(col->cid)) ||
    (type == Types::Range::META   && Types::MetaColumn::is_meta(col->cid))) {

    if(cid != rsp.cid || col->cid != cid) {
      SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING(cid no match) %s", 
                          rsp.to_string().c_str());
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

void Select::Scanner::locate_on_ranger(const EndPoints& endpoints, 
                                       bool next_range) {
  ++col->selector->result->completion;

  Protocol::Rgr::Params::RangeLocateReq params(cid, rid);
  if(next_range) {
    params.flags |= Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(range_offset);
    params.range_offset.insert(0, std::to_string(col->cid));
  }

  col->interval.apply_possible_range(params.range_begin, params.range_end);
  params.range_begin.insert(0, std::to_string(col->cid));
  params.range_end.insert(0, std::to_string(col->cid));
  if(type == Types::Range::MASTER && Types::MetaColumn::is_data(col->cid)) {
    auto meta_cid = Types::MetaColumn::get_meta_cid(col->col_seq);
    params.range_begin.insert(0, meta_cid);
    params.range_end.insert(0, meta_cid);
    if(next_range)
      params.range_offset.insert(0, meta_cid);
  }

  SWC_LOGF(LOG_DEBUG, "LocateRange-onRgr %s", params.to_string().c_str());

  Protocol::Rgr::Req::RangeLocate::request(
    params, endpoints,
    [next_range, profile=col->selector->result->profile.rgr_locate(type),
     scanner=shared_from_this()]
    (ReqBase::Ptr req, const Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      if(scanner->located_on_ranger(
          std::dynamic_pointer_cast<Protocol::Rgr::Req::RangeLocate>(req)->endpoints,
          req, rsp, next_range))
        --scanner->col->selector->result->completion;
    }
  );
}

bool Select::Scanner::located_on_ranger(
          const EndPoints& endpoints, 
          const ReqBase::Ptr& base, 
          const Protocol::Rgr::Params::RangeLocateRsp& rsp, 
          bool next_range) {

  SWC_LOGF(LOG_DEBUG, "Located-onRgr %s", rsp.to_string().c_str());
  if(rsp.err) {
    if(rsp.err == Error::RANGE_NOT_FOUND && 
       (next_range || type == Types::Range::META)) {
      --col->selector->result->completion;
      col->next_call(true);
      return false;
    }

    SWC_LOGF(LOG_DEBUG, "Located-oRgr RETRYING %s", 
                        rsp.to_string().c_str());                      
    if(rsp.err == Error::RS_NOT_LOADED_RANGE || 
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
    SWC_LOGF(LOG_DEBUG, "Located-onRgr RETRYING(no rid) %s", 
                        rsp.to_string().c_str());
    Env::Clients::get()->rangers.remove(cid, rid);
    parent->request_again();
    return false;
  }
  if(type == Types::Range::DATA && rsp.cid != col->cid) {
    SWC_LOGF(LOG_DEBUG, "Located-onRgr RETRYING(cid no match) %s",
              rsp.to_string().c_str());        
    Env::Clients::get()->rangers.remove(cid, rid);
    parent->request_again();
    return false;
  }

  if(type != Types::Range::DATA) {
    col->add_call([endpoints, scanner=std::make_shared<Scanner>(
      type, cid, col,
      parent, &rsp.range_begin, rid
      )] () { scanner->locate_on_ranger(endpoints, true); }
    );
  }

  std::make_shared<Scanner>(
    type == Types::Range::MASTER ? Types::Range::META : Types::Range::DATA,
    rsp.cid, col, 
    parent, nullptr, rsp.rid
  )->resolve_on_manager();

  return true;
}

void Select::Scanner::select(EndPoints endpoints, uint64_t rid, 
                             const ReqBase::Ptr& base) {
  ++col->selector->result->completion;

  Protocol::Rgr::Req::RangeQuerySelect::request(
    Protocol::Rgr::Params::RangeQuerySelectReq(
      col->cid, rid, col->interval
    ), 
    endpoints, 
    [rid, base, 
     profile=col->selector->result->profile.rgr_data(), 
     scanner=shared_from_this()] 
    (ReqBase::Ptr req, const Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
      profile.add(rsp.err);
      
      if(rsp.err) {
        SWC_LOGF(LOG_DEBUG, "Select RETRYING %s", rsp.to_string().c_str());
        if(rsp.err == Error::RS_NOT_LOADED_RANGE || 
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
        if(rsp.reached_limit) {
          scanner->select(
            std::dynamic_pointer_cast<Protocol::Rgr::Req::RangeQuerySelect>(req)
              ->endpoints, 
            rid, base
          );
        } else {
          col->next_call();
        }
      }

      if(rsp.data.size)
        col->selector->response_partials();
          
      if(!--col->selector->result->completion)
        col->selector->response();
    },

    col->selector->timeout
  );
}

  
  
}}}
