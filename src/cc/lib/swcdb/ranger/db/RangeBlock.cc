/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/sys/Resources.h"
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
              m_key_end(interval.key_end),  
              m_cells(
                DB::Cells::Mutable(
                  blocks->range->cfg->key_seq, 
                  blocks->range->cfg->cell_versions(), 
                  blocks->range->cfg->cell_ttl(), 
                  blocks->range->cfg->column_type())),
              m_state(state), m_processing(0) {
}

Block::~Block() { }

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

bool Block::is_consist(const DB::Cells::Interval& intval) const {
  return 
    (intval.key_end.empty() || m_prev_key_end.empty() ||
     DB::KeySeq::compare(m_cells.key_seq, m_prev_key_end, intval.key_end)
      == Condition::GT)
    &&
    (intval.key_begin.empty() || is_in_end(intval.key_begin));
}

bool Block::is_in_end(const DB::Cell::Key& key) const {
  std::shared_lock lock(m_mutex);
  return _is_in_end(key);
}

bool Block::_is_in_end(const DB::Cell::Key& key) const {
  return m_key_end.empty() || (!key.empty() && 
          DB::KeySeq::compare(m_cells.key_seq, m_key_end, key) 
                                              != Condition::GT);
}

bool Block::is_next(const DB::Specs::Interval& spec) const {
  if(includes_end(spec)) {
    std::shared_lock lock(m_mutex);
    return (spec.offset_key.empty() || _is_in_end(spec.offset_key)) && 
            _includes_begin(spec);
  }
  return false;
}

bool Block::includes(const DB::Specs::Interval& spec) const {
  if(includes_end(spec)) {
    std::shared_lock lock(m_mutex);
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
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [this](){ scan(std::make_shared<ReqScanBlockLoader>());}
  );
}

bool Block::add_logged(const DB::Cells::Cell& cell) {
  if(!is_in_end(cell.key))
    return false;
  
  if(loaded()) {
    std::scoped_lock lock(m_mutex);
    m_cells.add_raw(cell);
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

  while(remain) {
    ++count;
    try {
      cell.read(&buf, &remain);
      
    } catch(...) {
      SWC_LOGF(LOG_ERROR, 
        "Cell trunclated at count=%llu/%llu remain=%llu %s < key <= %s",
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
      
    if(++added % 100 == 0 && splitter()) {
      was_splitted = true;
      offset_hint = 0;
    }
  }
  return added;
}

SWC_SHOULD_INLINE
bool Block::splitter() {
  return _need_split() && blocks->_split(ptr(), false);
}

bool Block::scan(const ReqScan::Ptr& req) {
  bool loaded;
  {
    Mutex::scope lock(m_mutex_state);

    if(!(loaded = m_state == State::LOADED)) {
      m_queue.push({.req=req, .ts=Time::now_ns()});
      if(m_state != State::NONE)
        return true;
      m_state = State::LOADING;
    }
  }

  if(loaded) switch(req->type) {
    case ReqScan::Type::BLK_PRELOAD: {
      processing_decrement();
      return false;
    }
    default:
      return _scan(req, true);
  }

  auto loader = new BlockLoader(ptr());
  loader->run();
  return true;
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

  m_cells.split(blk->m_cells, m_key_end, blk->m_key_end, loaded);

  _add(blk);
  return blk;
}

void Block::_add(Block::Ptr blk) {
  blk->prev = ptr();
  if(next) {
    blk->next = next;
    next->prev = blk;
  }
  blk->_set_prev_key_end(m_key_end);
  next = blk;
}

SWC_SHOULD_INLINE
void Block::_set_prev_key_end(const DB::Cell::Key& key) {
  m_prev_key_end.copy(key);
}

SWC_SHOULD_INLINE
Condition::Comp Block::_cond_key_end(const DB::Cell::Key& key) const {
  return DB::KeySeq::compare(m_cells.key_seq, m_key_end, key);
}

SWC_SHOULD_INLINE
void Block::_set_key_end(const DB::Cell::Key& key) {
  m_key_end.copy(key);
}

size_t Block::release() {
  size_t released = 0;
  if(!m_processing && m_mutex.try_lock()) {
    Mutex::scope lock(m_mutex_state);
    if(!m_processing && m_state == State::LOADED) {
      m_state = State::NONE;
      released += _size_bytes();
      m_cells.free();
    }
    m_mutex.unlock();
  }
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
  return _size_bytes();
}
  
SWC_SHOULD_INLINE
size_t Block::_size_bytes() const {
  return m_cells.size_bytes() + m_cells.size() * m_cells._cell_sz;
}

/*
bool Block::need_split() {
  bool ok;
  if(ok = loaded() && ok = m_mutex.try_lock_shared()) {
    ok = _need_split();
    m_mutex.unlock_shared();
  }
  return ok;
}
*/

bool Block::_need_split() const {
  auto sz = _size();
  return sz > 1 && 
    (sz >= blocks->range->cfg->block_cells() * 2 || 
     _size_bytes() >= blocks->range->cfg->block_size() * 2) && 
    !m_cells.has_one_key();
}

SWC_SHOULD_INLINE
void Block::free_key_end() {
  m_key_end.free();
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
  
  if(m_mutex.try_lock()) {
    s.append(m_key_end.to_string());
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

bool Block::_scan(const ReqScan::Ptr& req, bool synced) {
  // m_key_end incl. req->spec // ?has-changed(split)
  uint64_t ts = Time::now_ns();
  {
    std::shared_lock lock(m_mutex);
    m_cells.scan(req.get());
  }
  req->profile.add_block_scan(ts);

  processing_decrement();

  if(req->reached_limits()) {
    int err = Error::OK;
    if(req->with_block())
      req->block = ptr();
    blocks->processing_decrement();
    req->response(err);
    return true;
  }

  if(req->release_block)
    release();

  if(!synced)
    blocks->scan(req, ptr());
  
  return false;
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
          asio::post(*Env::IoCtx::io()->ptr(),
                     [this, req=q.req]() { _scan(req); } );
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