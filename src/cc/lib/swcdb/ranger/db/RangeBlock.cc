/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/ranger/db/RangeBlock.h"
#include "swcdb/ranger/db/RangeBlocks.h"


namespace SWC { namespace Ranger {


SWC_SHOULD_INLINE
Block::Ptr Block::make(const DB::Cells::Interval& interval,
                       Blocks* blocks, State state) {
  return new Block(interval, blocks, state);
}

Block::Block(const DB::Cells::Interval& interval, 
             Blocks* blocks, State state)
            : blocks(blocks), next(nullptr), prev(nullptr),
              m_cells(
                DB::Cells::Mutable(
                  blocks->range->cfg->key_seq, 
                  blocks->range->cfg->cell_versions(), 
                  blocks->range->cfg->cell_ttl(), 
                  blocks->range->cfg->column_type())),
              m_key_end(interval.key_end),  
              m_state(state), m_processing(0) {
  RangerEnv::res().more_mem_usage(size_of());
}

Block::~Block() {
  RangerEnv::res().less_mem_usage(
    size_of() +
    (m_cells.empty() ? 0 : m_cells.size_of_internal())
  );
}

size_t Block::size_of() const {
  return sizeof(*this);
}

SWC_SHOULD_INLINE
Block::Ptr Block::ptr() {
  return this;
}

void Block::schema_update() {
  std::scoped_lock lock(m_mutex);
  m_cells.configure(
    blocks->range->cfg->cell_versions(), 
    blocks->range->cfg->cell_ttl(), 
    blocks->range->cfg->column_type()
  );
}

SWC_SHOULD_INLINE
void Block::set_prev_key_end(const DB::Cell::Key& key) {
  m_prev_key_end.copy(key);
}

Condition::Comp Block::cond_key_end(const DB::Cell::Key& key) const {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return DB::KeySeq::compare(m_cells.key_seq, m_key_end, key);
}

void Block::set_key_end(const DB::Cell::Key& key) {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  m_key_end.copy(key);
}

void Block::free_key_end() {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  m_key_end.free();
}

void Block::get_key_end(DB::Cell::Key& key) const {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  key.copy(m_key_end);
}

bool Block::is_consist(const DB::Cells::Interval& intval) const {
  return 
    (intval.key_end.empty() || m_prev_key_end.empty() ||
     DB::KeySeq::compare(m_cells.key_seq, m_prev_key_end, intval.key_end)
      == Condition::GT)
    &&
    (intval.key_begin.empty() || is_in_end(intval.key_begin));
}

bool Block::is_in_end(const DB::Cell::Key& key) const {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return _is_in_end(key);
}

bool Block::_is_in_end(const DB::Cell::Key& key) const {
  return m_key_end.empty() || (!key.empty() && 
          DB::KeySeq::compare(m_cells.key_seq, m_key_end, key) 
                                              != Condition::GT);
}

bool Block::is_next(const DB::Specs::Interval& spec) const {
  if(includes_end(spec)) {
    LockAtomic::Unique::scope lock(m_mutex_intval);
    return (spec.offset_key.empty() || _is_in_end(spec.offset_key)) && 
            _includes_begin(spec);
  }
  return false;
}

bool Block::includes(const DB::Specs::Interval& spec) const {
  if(includes_end(spec)) {
    LockAtomic::Unique::scope lock(m_mutex_intval);
    return _includes_begin(spec);
  }
  return false;
}

bool Block::_includes_begin(const DB::Specs::Interval& spec) const {
  return m_key_end.empty() || 
         spec.is_matching_begin(m_cells.key_seq, m_key_end);
}

bool Block::includes_end(const DB::Specs::Interval& spec) const {
  return m_prev_key_end.empty() ||
         spec.is_matching_end(m_cells.key_seq, m_prev_key_end);
}
    
void Block::preload() {
  Env::IoCtx::post([this](){ scan(std::make_shared<ReqScanBlockLoader>());} );
}

bool Block::add_logged(const DB::Cells::Cell& cell) {
  if(!is_in_end(cell.key))
    return false;
  
  if(loaded()) {
    std::scoped_lock lock(m_mutex);
    ssize_t sz = m_cells.size_of_internal();
    m_cells.add_raw(cell);
    RangerEnv::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);
    splitter();
  }
  return true;
}
  
void Block::load_cells(const DB::Cells::MutableVec& vec_cells) {
  if(!vec_cells.empty()) {
    std::scoped_lock lock(m_mutex);
    for(auto cells : vec_cells) {
      if(!cells->scan_after(m_prev_key_end, m_key_end, m_cells))
        break;
      splitter();
    }
  }
}

size_t Block::load_cells(const uint8_t* buf, size_t remain, 
                         uint32_t revs, size_t avail,
                         bool& was_splitted, bool synced) {
  DB::Cells::Cell cell;
  size_t count = 0;
  size_t added = 0;
  size_t offset_hint = 0;

  std::scoped_lock lock(m_mutex);
  if(revs > blocks->range->cfg->cell_versions())
    // schema change from more to less results in dups
    synced = false;
  else if(!synced && m_cells.empty())
    synced = true;
  ssize_t sz = m_cells.size_of_internal();

  while(remain) {
    ++count;
    try {
      cell.read(&buf, &remain);
      
    } catch(...) {
      SWC_LOGF(LOG_ERROR, 
        "Cell trunclated at count=%lu/%lu remain=%lu %s < key <= %s",
        count, avail, remain, 
        m_prev_key_end.to_string().c_str(), m_key_end.to_string().c_str());
      break;
    }
    
    if(!m_prev_key_end.empty() &&  
        DB::KeySeq::compare(m_cells.key_seq, m_prev_key_end, cell.key) 
          != Condition::GT)
      continue;
    
    if(!m_key_end.empty() && 
        DB::KeySeq::compare(m_cells.key_seq, m_key_end, cell.key)
          == Condition::GT)
      break;

    if(cell.has_expired(m_cells.ttl))
      continue;

    if(synced)
      m_cells.add_sorted(cell);
    else
      m_cells.add_raw(cell, &offset_hint);
      
    if(++added % 1000 == 0) {
      RangerEnv::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);
      if(splitter()) {
        was_splitted = true;
        offset_hint = 0;
      }
      sz = m_cells.size_of_internal();
    }
  }
  RangerEnv::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);
  return added;
}

SWC_SHOULD_INLINE
bool Block::splitter() {
  return _need_split() && blocks->_split(ptr(), false);
}

Block::ScanState Block::scan(const ReqScan::Ptr& req) {
  bool loaded;
  {
    Mutex::scope lock(m_mutex_state);

    if(!(loaded = m_state == State::LOADED)) {
      m_queue.push({.req=req, .ts=Time::now_ns()});
      if(m_state != State::NONE)
        return ScanState::QUEUED;
      m_state = State::LOADING;
    }
  }

  if(loaded) switch(req->type) {
    case ReqScan::Type::BLK_PRELOAD: {
      processing_decrement();
      return ScanState::RESPONDED;
    }
    default:
      return _scan(req, true);
  }

  auto loader = new BlockLoader(ptr());
  loader->run();
  return ScanState::QUEUED;
}

void Block::loaded(int err, const BlockLoader* loader) {
  {
    Mutex::scope lock(m_mutex_state);
    m_state = State::LOADED;
  }

  if(err) {
    SWC_LOGF(LOG_ERROR, "Block::loaded err=%d(%s)", err, Error::get_text(err));
    quick_exit(1); // temporary halt
    //run_queue(err);
    return;
  }
  run_queue(err);

  delete loader;
}

Block::Ptr Block::split(bool loaded) {
  Block::Ptr blk = nullptr;
  if(m_mutex.try_lock()) {
    blk = _split(loaded);
    m_mutex.unlock();
  }
  return blk;
}

Block::Ptr Block::_split(bool loaded) {
  Block::Ptr blk = Block::make(
    DB::Cells::Interval(m_cells.key_seq), 
    blocks,
    loaded ? State::LOADED : State::NONE
  );
  ssize_t sz = loaded ? 0 : m_cells.size_of_internal();
  m_cells.split(blk->m_cells, loaded);
  {
    LockAtomic::Unique::scope lock(m_mutex_intval);
    blk->m_key_end.copy(m_key_end);
    m_key_end.copy(m_cells.back()->key);
  }
  if(sz)
    RangerEnv::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);

  _add(blk);
  return blk;
}

void Block::_add(Block::Ptr blk) {
  blk->prev = ptr();
  if(next) {
    blk->next = next;
    next->prev = blk;
  }
  get_key_end(blk->m_prev_key_end);
  next = blk;
}

size_t Block::release() {
  size_t released = 0;
  if(!m_processing && m_mutex.try_lock()) {
    Mutex::scope lock(m_mutex_state);
    if(!m_processing && m_state == State::LOADED) {
      m_state = State::NONE;
      released += m_cells.size_of_internal();
      m_cells.free();
    }
    m_mutex.unlock();
  }
  if(released)
    RangerEnv::res().less_mem_usage(released);
  return released;
}

SWC_SHOULD_INLINE
void Block::processing_increment() {
  ++m_processing;
}

SWC_SHOULD_INLINE
void Block::processing_decrement() {
  --m_processing;
}

bool Block::removed() {
  Mutex::scope lock(m_mutex_state);
  return m_state == State::REMOVED;
}

bool Block::loaded() {
  Mutex::scope lock(m_mutex_state);
  return m_state == State::LOADED;
}

bool Block::need_load() {
  Mutex::scope lock(m_mutex_state);
  return m_state == State::NONE;
}

bool Block::processing() const {
  return m_processing;
}

size_t Block::size() {
  std::shared_lock lock(m_mutex);
  return _size();
}

SWC_SHOULD_INLINE
size_t Block::_size() const {
  return m_cells.size();
}
  
size_t Block::size_bytes() {
  std::shared_lock lock(m_mutex);
  return m_cells.size_bytes();
}
  
size_t Block::size_of_internal() {
  std::shared_lock lock(m_mutex);
  return m_cells.size_of_internal();
}

bool Block::_need_split() const {
  auto sz = _size();
  return sz > 1 && 
    (sz >= blocks->range->cfg->block_cells() * 2 || 
     m_cells.size_bytes() >= blocks->range->cfg->block_size() * 2) && 
    !m_cells.has_one_key();
}

std::string Block::to_string() {
  std::string s("Block(state=");
  {
    Mutex::scope lock(m_mutex_state);
    s.append(std::to_string((uint8_t)m_state));
  }
  s.append(" ");
  s.append(Types::to_string(m_cells.key_seq));
  s.append(" ");
  s.append(m_prev_key_end.to_string());
  s.append(" < key <= ");
  {
    LockAtomic::Unique::scope lock(m_mutex_intval);
    s.append(m_key_end.to_string());
  }

  if(m_mutex.try_lock()) {
    s.append(" ");
    s.append(m_cells.to_string());
    m_mutex.unlock();
  } else {
    s.append("CellsLocked");
  }

  s.append(" queue=");
  s.append(std::to_string(m_queue.size()));

  s.append(" processing=");
  s.append(std::to_string(m_processing));
  s.append(")");
  return s;
}

Block::ScanState Block::_scan(const ReqScan::Ptr& req, bool synced) {
  // if(is_next(req->spec)) // ?has-changed(split)
  int err = Error::OK;
  uint64_t ts = Time::now_ns();
  try {
    std::shared_lock lock(m_mutex);
    m_cells.scan(req.get());

  } catch (...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  req->profile.add_block_scan(ts);

  processing_decrement();

  if(err || req->reached_limits()) {
    if(req->with_block())
      req->block = ptr();
    blocks->processing_decrement();
    req->response(err);
    return ScanState::RESPONDED;
  }

  if(req->release_block)
    release();

  if(!synced)
    blocks->scan(req, ptr());
  
  return ScanState::SYNCED;
}

void Block::run_queue(int& err) {
  do { 
    ReqQueue q(std::move(m_queue.front()));
    switch(q.req->type) {
      case ReqScan::Type::BLK_PRELOAD: {
        processing_decrement();
        break;
      }

      default: {
        if(!err) {
          q.req->profile.add_block_load(q.ts);
          Env::IoCtx::post([this, req=q.req]() { _scan(req); } );
          break;
        }
        blocks->processing_decrement();
        processing_decrement();
        q.req->response(err);
      }
    }
  } while(m_queue.pop_and_more());
}



}}