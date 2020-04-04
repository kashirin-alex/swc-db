/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/sys/Resources.h"
#include "swcdb/ranger/db/RangeBlock.h"
#include "swcdb/ranger/db/RangeBlocks.h"


namespace SWC { namespace Ranger {


Block::Ptr Block::make(const DB::Cells::Interval& interval,
                       Blocks* blocks, State state) {
  return new Block(interval, blocks, state);
}

Block::Block(const DB::Cells::Interval& interval, 
             Blocks* blocks, State state)
            : m_interval(interval),  
              m_cells(
                DB::Cells::Mutable(
                  blocks->range->cfg->cell_versions(), 
                  blocks->range->cfg->cell_ttl(), 
                  blocks->range->cfg->column_type())),
              blocks(blocks), 
              m_state(state), m_processing(0), 
              next(nullptr), prev(nullptr) {
}

Block::~Block() { }

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
  //std::shared_lock lock(m_mutex);
  // m_prev_key_end && m_interval.key_end behave as const
  return 
    (intval.key_begin.empty() || m_interval.is_in_end(intval.key_begin))
    && 
    (intval.key_end.empty() || m_prev_key_end.empty() ||
     m_prev_key_end.compare(intval.key_end) == Condition::GT);
}

bool Block::is_in_end(const DB::Cell::Key& key) const {
  //std::shared_lock lock(m_mutex);
  return m_interval.is_in_end(key);
}

bool Block::is_next(const DB::Specs::Interval& spec) {
  //std::shared_lock lock(m_mutex);
  return (spec.offset_key.empty() || m_interval.is_in_end(spec.offset_key))
          && includes(spec);
}

bool Block::includes(const DB::Specs::Interval& spec) {
  bool ok;
  if(ok = m_interval.includes_end(spec)) {
    std::shared_lock lock(m_mutex);
    ok = m_interval.includes_begin(spec);
  }
  return ok; 
}
    
void Block::preload() {
  //SWC_PRINT << " BLK_PRELOAD " << to_string() << SWC_PRINT_CLOSE;
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [ptr=ptr()](){ 
      ptr->scan(std::make_shared<ReqScan>(ReqScan::Type::BLK_PRELOAD));
    }
  );
}

bool Block::add_logged(const DB::Cells::Cell& cell) {
  if(!is_in_end(cell.key))
    return false;
  
  if(!loaded()) 
    return true;

  std::scoped_lock lock(m_mutex);

  m_cells.add_raw(cell);
  if(!m_interval.is_in_begin(cell.key)) {
    m_interval.key_begin.copy(cell.key); 
  //m_interval.expand(cell.timestamp);
  }
  splitter();
  return true;
}
  
void Block::load_cells(const DB::Cells::Mutable& cells) {
  if(cells.empty())
    return;

  auto ts = Time::now_ns();

  std::scoped_lock lock(m_mutex);
  size_t added = m_cells.size();
    
  cells.scan(m_interval, m_cells);

  if(!m_cells.empty() && !m_interval.key_begin.empty())
    m_cells.expand_begin(m_interval);

  added = m_cells.size() - added;
  auto took = Time::now_ns() - ts;
  SWC_PRINT << "Block::load_cells(cells)"
            << " synced=0"
            << " avail=" << cells.size() 
            << " added=" << added 
            << " skipped=" << cells.size()-added
            << " avg=" << (added>0 ? took / added : 0)
            << " took=" << took
            << std::flush << " " << m_cells.to_string() 
            << SWC_PRINT_CLOSE;
}

size_t Block::load_cells(const uint8_t* buf, size_t remain, 
                         uint32_t revs, size_t avail,
                         bool& was_splitted, bool synced) {
  DB::Cells::Cell cell;
  size_t count = 0;
  size_t added = 0;
    
  uint64_t ts = Time::now_ns();

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
      
    } catch(std::exception) {
      SWC_LOGF(LOG_ERROR, "Cell trunclated at count=%llu/%llu remain=%llu, %s",
               count, avail, remain, m_interval.to_string().c_str());
      break;
    }
    
    if(!m_prev_key_end.empty() &&  
        m_prev_key_end.compare(cell.key) != Condition::GT)
      continue;
    
    if(!m_interval.key_end.empty() && 
        m_interval.key_end.compare(cell.key) == Condition::GT)
      break;

    ++added;
    if(cell.has_expired(m_cells.ttl))
      continue;

    if(synced)
      m_cells.add_sorted(cell);
    else
      m_cells.add_raw(cell);
      
    if(added % 100 == 0 && splitter())
      was_splitted = true;
  }
    
  if(!m_cells.empty() && !m_interval.key_begin.empty())
    m_cells.expand_begin(m_interval);
    
  auto took = Time::now_ns() - ts;
  SWC_PRINT << "Block::load_cells(rbuf)"
            << " synced=" << synced 
            << " avail=" << avail 
            << " added=" << added 
            << " skipped=" << avail-added
            << " avg=" << (added>0 ? took / added : 0)
            << " took=" << took
            << " " << m_cells.to_string()
            << " splitted=" << was_splitted
            << SWC_PRINT_CLOSE;
  return added;
}

bool Block::splitter() {
  return _need_split() && blocks->_split(ptr(), false);
}

bool Block::scan(ReqScan::Ptr req) {
  bool loaded;
  {
    Mutex::scope lock(m_mutex_state);

    if(!(loaded = m_state == State::LOADED)) {
      m_queue.push(req);
      if(m_state != State::NONE)
        return true;
      m_state = State::LOADING;
    }
  }

  if(loaded) {
    if(req->type == ReqScan::Type::BLK_PRELOAD) {
      processing_decrement();
      return false;
    }
    return _scan(req, true);
  }

  
  auto loader = new BlockLoader(ptr());
  loader->run();
  return true;
}

void Block::loaded(int err) {
  {
    Mutex::scope lock(m_mutex_state);
    m_state = State::LOADED;
  }
  
  if(err) {
    SWC_LOGF(LOG_ERROR, "Block::loaded err=%d(%s)", err, Error::get_text(err));
    quick_exit(1); // temporary halt
    run_queue(err);
    return;
  }
  run_queue(err);
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
    DB::Cells::Interval(), 
    blocks,
    loaded ? State::LOADED : State::NONE
  );

  m_cells.split(
    m_cells.size()/2, //blocks->range->cfg->block_cells(), 
    blk->m_cells,
    m_interval,
    blk->m_interval,
    loaded
  );

  _add(blk);
  return blk;
}

void Block::_add(Block::Ptr blk) {
  blk->prev = ptr();
  if(next) {
    blk->next = next;
    next->prev = blk;
  }
  blk->_set_prev_key_end(m_interval.key_end);
  next = blk;
}

void Block::_set_prev_key_end(const DB::Cell::Key& key) {
  m_prev_key_end.copy(key);
}

Condition::Comp Block::_cond_key_end(const DB::Cell::Key& key) const {
  return m_interval.key_end.compare(key);
}

void Block::_set_key_end(const DB::Cell::Key& key) {
  m_interval.key_end.copy(key);
}

/*
void Block::expand_next_and_release(DB::Cell::Key& key_begin) {
  std::scoped_lock lock(m_mutex);
  Mutex::scope lock(m_mutex_state);

  m_state = State::REMOVED;
  key_begin.copy(m_interval.key_begin);
  m_cells.free();
  m_interval.free();
}

void Block::merge_and_release(Block::Ptr blk) {
  std::scoped_lock lock(m_mutex);
  Mutex::scope lock(m_mutex_state);

  m_state = State::NONE;
  blk->expand_next_and_release(m_interval.key_begin);
  m_cells.free();
}
*/

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

void Block::processing_increment() {
  ++m_processing;
}

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

size_t Block::_size() const {
  return m_cells.size();
}
  
size_t Block::size_bytes() {
  std::shared_lock lock(m_mutex);
  return _size_bytes();
}
  
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

void Block::free_key_begin() {
  std::scoped_lock lock(m_mutex);
  m_interval.key_begin.free();
}

void Block::free_key_end() {
  std::scoped_lock lock(m_mutex);
  m_interval.key_end.free();
}

std::string Block::to_string() {
  std::shared_lock lock1(m_mutex);
  Mutex::scope lock2(m_mutex_state);

  std::string s("Block(state=");
  s.append(std::to_string((uint8_t)m_state));

  s.append(" prev=");
  s.append(m_prev_key_end.to_string());
  s.append(" ");
  s.append(m_interval.to_string());

  s.append(" ");
  s.append(m_cells.to_string());

  s.append(" queue=");
  s.append(std::to_string(m_queue.size()));

  s.append(" processing=");
  s.append(std::to_string(m_processing));
  s.append(")");
  return s;
}

bool Block::_scan(ReqScan::Ptr req, bool synced) {
  {
    size_t skips = 0; // Ranger::Stats
    std::shared_lock lock(m_mutex);
    //if(m_interval.includes(req->spec, true)) {
    m_cells.scan(
      req->spec, 
      req->cells, 
      req->offset,
      [req]() { return req->reached_limits(); },
      skips, 
      req->selector()
    );
    //}
  }

  processing_decrement();

  if(req->reached_limits()) {
    blocks->processing_decrement();
    int err = Error::OK;
    req->response(err);
    return true;
  } else if(!synced) {
    blocks->scan(req, ptr());
  }
  return false;
}

void Block::run_queue(int& err) {
  ReqScan::Ptr req;
  do {

    if((req = m_queue.front())->type == ReqScan::Type::BLK_PRELOAD) {
      processing_decrement();

    } else if(err) {
      blocks->processing_decrement();
      processing_decrement();
      req->response(err);
          
    } else {
      asio::post(*Env::IoCtx::io()->ptr(), [this, req]() { _scan(req); } );
    }

  } while(m_queue.pop_and_more());
}



}}