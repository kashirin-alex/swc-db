/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/sys/Resources.h"
#include "swcdb/db/Files/RangeBlock.h"
#include "swcdb/db/Files/RangeBlocks.h"


namespace SWC { namespace Files { namespace Range {


Block::Ptr Block::make(const DB::Cells::Interval& interval,
                       Blocks* blocks, State state) {
  return new Block(interval, blocks, state);
}

Block::Block(const DB::Cells::Interval& interval, 
             Blocks* blocks, State state)
            : m_interval(interval),  
              m_cells(
                DB::Cells::Mutable(
                  0, blocks->range->cfg->cell_versions(), 
                  blocks->range->cfg->cell_ttl(), 
                  blocks->range->cfg->column_type())),
              m_blocks(blocks), 
              m_state(state), m_processing(0), 
              next(nullptr), prev(nullptr), m_load_require(0) {
}

Block::~Block() { }

Block::Ptr Block::ptr() {
  return this;
}

const bool Block::is_consist(const DB::Cells::Interval& intval) {
  std::shared_lock lock(m_mutex);
  return 
    (intval.key_begin.empty() || m_interval.is_in_end(intval.key_begin))
    && 
    (intval.key_end.empty() || 
      !prev || m_prev_key_end.compare(intval.key_end) == Condition::GT);
}

const bool Block::is_in_end(const DB::Cell::Key& key) {
  std::shared_lock lock(m_mutex);
  return m_interval.is_in_end(key);
}

const bool Block::is_next(const DB::Specs::Interval& spec) {
  std::shared_lock lock(m_mutex);
  return (spec.offset_key.empty() || m_interval.is_in_end(spec.offset_key))
          && m_interval.includes(spec);
}

const bool Block::includes(const DB::Specs::Interval& spec) {
  std::shared_lock lock(m_mutex);
  return m_interval.includes(spec);
}
    
void Block::preload() {
  //std::cout << " BLK_PRELOAD " << to_string() << "\n";
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [ptr=ptr()](){ 
      ptr->scan(
        std::make_shared<DB::Cells::ReqScan>(
          DB::Cells::ReqScan::Type::BLK_PRELOAD)
      );
    }
  );
}

const bool Block::add_logged(const DB::Cells::Cell& cell) {
  {
    std::shared_lock lock(m_mutex);
      
    if(!m_interval.is_in_end(cell.key))
      return false;

    if(!loaded())
      return cell.on_fraction ? 
        m_interval.key_end.compare(cell.key, cell.on_fraction) 
        != Condition::GT : true;
  }

  std::scoped_lock lock(m_mutex);
  m_cells.add(cell);

  if(cell.on_fraction)
    // return added for fraction under current end
    return m_interval.key_end.compare(
      cell.key, cell.on_fraction) != Condition::GT;
      
  if(!m_interval.is_in_begin(cell.key)) {
    m_interval.key_begin.copy(cell.key); 
    m_interval.expand(cell.timestamp);
  }
  return true;
}
  
void Block::load_cells(const DB::Cells::Mutable& cells) {
  std::scoped_lock lock(m_mutex);
  auto ts = Time::now_ns();
  size_t added = m_cells.size();
    
  if(cells.size())
    cells.scan(m_interval, m_cells);
    
  if(m_cells.size() && !m_interval.key_begin.empty())
    m_cells.expand_begin(m_interval);

  added = m_cells.size() - added;
  auto took = Time::now_ns()-ts;
  std::cout << "Block::load_cells(cells)"
            << " synced=0"
            << " avail=" << cells.size() 
            << " added=" << added 
            << " skipped=" << cells.size()-added
            << " avg=" << (added>0 ? took / added : 0)
            << " took=" << took
            << std::flush << " " << m_cells.to_string() << "\n";
}

const size_t Block::load_cells(const uint8_t* buf, size_t remain, 
                               size_t avail, bool& was_splitted) {
  auto ts = Time::now_ns();
  DB::Cells::Cell cell;
  size_t count = 0;
  size_t added = 0;
    
  const uint8_t** rbuf = &buf;
  size_t* remainp = &remain;

  std::scoped_lock lock(m_mutex);
  bool synced = !m_cells.size();
    
  while(remain) {
    try {
      cell.read(rbuf, remainp);
      count++;
    } catch(std::exception) {
      SWC_LOGF(LOG_ERROR, 
        "Cell trunclated at count=%llu remain=%llu %s, %s", 
        count, avail-count, 
        cell.to_string().c_str(),  m_interval.to_string().c_str());
      break;
    }
      
    if(prev && m_prev_key_end.compare(cell.key) != Condition::GT)
      continue;
    if(!m_interval.key_end.empty() 
        && m_interval.key_end.compare(cell.key) == Condition::GT)
      break;

    if(synced)
      m_cells.push_back(cell);
    else
      m_cells.add(cell);
      
    added++;

    if(splitter() && !was_splitted)
      was_splitted = true;
  }
    
  if(m_cells.size() && !m_interval.key_begin.empty())
    m_cells.expand_begin(m_interval);
    
  auto took = Time::now_ns()-ts;
  std::cout << "Block::load_cells(rbuf)"
            << " synced=" << synced 
            << " avail=" << avail 
            << " added=" << added 
            << " skipped=" << avail-added
            << " avg=" << (added>0 ? took / added : 0)
            << " took=" << took
            << std::flush << " " << m_cells.to_string() << "\n";
             
  return added;
}

const bool Block::splitter() {
  return m_blocks->_split(ptr(), false);
}

void Block::loaded_cellstores(int err) {  
  bool loaded;
  {
    std::scoped_lock lock(m_mutex_state);
    if(loaded = ++m_load_require == 2)
      m_state = State::LOADED;
  }
  if(err) {
    SWC_LOGF(LOG_ERROR, "cellstores-load_cells %s err=%d",
                        m_blocks->cellstores.to_string().c_str(), err);
    quick_exit(1); // temporary halt
    run_queue(err);
    return;
  }
  if(loaded)
    run_queue(err);

  //m_blocks->commitlog.load_cells(ptr());
}

void Block::loaded_logs(int err) {
  bool loaded;
  {
    std::scoped_lock lock(m_mutex_state);
    if(loaded = ++m_load_require == 2)
      m_state = State::LOADED;
  }
  if(err) {
    SWC_LOGF(LOG_ERROR, "commitlog-load_cells %s err=%d", 
                         m_blocks->commitlog.to_string().c_str(), err)  
    quick_exit(1); // temporary halt
  }
  if(loaded)
    run_queue(err);
}

const bool Block::scan(DB::Cells::ReqScan::Ptr req) {
  bool loaded;
  {
    std::scoped_lock lock(m_mutex_state);

    if(!(loaded = m_state == State::LOADED)) {
      m_queue.push(req);
      if(m_state != State::NONE)
        return true;
      m_state = State::LOADING;
    }
  }

  if(loaded) {
    if(req->type == DB::Cells::ReqScan::Type::BLK_PRELOAD) {
      processing_decrement();
      return false;
    }
    return _scan(req, true);
  }
  
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [ptr=ptr()]() { ptr->m_blocks->cellstores.load_cells(ptr); } 
  );
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [ptr=ptr()]() { ptr->m_blocks->commitlog.load_cells(ptr); } 
  );
  
  return true;
}

Block::Ptr Block::split(bool loaded) {
  Block::Ptr blk = nullptr;
  if(!m_mutex.try_lock())
    return blk;
  blk = _split(loaded);
  m_mutex.unlock();
  return blk;
}
    
Block::Ptr Block::_split(bool loaded) {
  Block::Ptr blk = Block::make(
    DB::Cells::Interval(), 
    m_blocks,
    loaded ? State::LOADED : State::NONE
  );

  m_cells.move_from_key_offset(
    m_blocks->range->cfg->block_cells(), 
    blk->m_cells
  );
  assert(m_cells.size());
  assert(blk->m_cells.size());

  blk->m_cells.expand_begin(blk->m_interval);
  blk->m_interval.set_key_end(m_interval.key_end);
  blk->m_interval.set_ts_latest(m_interval.ts_latest);
      
  m_interval.key_end.free();
  m_interval.ts_latest.free();
  m_cells.expand_end(m_interval);
    
  if(!loaded)
    blk->m_cells.free();
      
  add(blk);
  return blk;
}

void Block::add(Block::Ptr blk) {
  //std::scoped_lock lock(m_mutex_state);
  blk->prev = ptr();
  if(next) {
    blk->next = next;
    next->prev = blk;
  }
  blk->m_prev_key_end.copy(m_interval.key_end);
  next = blk;
  // blk->set_prev_end()
}

/*
void Block::expand_next_and_release(DB::Cell::Key& key_begin) {
  std::scoped_lock lock(m_mutex, m_mutex_state);

  m_state = State::REMOVED;
  key_begin.copy(m_interval.key_begin);
  m_cells.free();
  m_interval.free();
}

void Block::merge_and_release(Block::Ptr blk) {
  std::scoped_lock lock(m_mutex, m_mutex_state);

  m_state = State::NONE;
  blk->expand_next_and_release(m_interval.key_begin);
  m_cells.free();
}
*/

const size_t Block::release() {
  size_t released = 0;
  if(m_processing || !loaded())
    return released;

  std::scoped_lock lock(m_mutex, m_mutex_state);

  m_state = State::NONE;
  released += m_cells.size();
  m_cells.free();
  m_load_require = 0;
  return released;
}

void Block::processing_increment() {
  m_processing++;
}

void Block::processing_decrement() {
  m_processing--;
}

const bool Block::removed() {
  std::shared_lock lock(m_mutex_state);
  return m_state == State::REMOVED;
}

const bool Block::loaded() {
  std::shared_lock lock(m_mutex_state);
  return m_state == State::LOADED;
}

const bool Block::processing() const {
  return m_processing;
}

const size_t Block::size() {
  std::shared_lock lock(m_mutex);
  return m_cells.size();
}

const size_t Block::_size() {
  return m_cells.size();
}
  
const size_t Block::size_bytes() {
  std::shared_lock lock(m_mutex);
  return m_cells.size_bytes() + m_cells.size() * m_cells._cell_sz;
}

void Block::free_key_begin() {
  std::scoped_lock lock(m_mutex);
  m_interval.key_begin.free();
}

void Block::free_key_end() {
  std::scoped_lock lock(m_mutex);
  m_interval.key_end.free();
}

const std::string Block::to_string() {
  std::shared_lock lock1(m_mutex_state);
  std::shared_lock lock2(m_mutex);

  std::string s("Block(state=");
  s.append(std::to_string((uint8_t)m_state));
      
  s.append(" ");
  s.append(m_interval.to_string());

  s.append(" ");
  s.append(m_cells.to_string());
  s.append(" ");
  if(m_cells.size()) {
    DB::Cells::Cell cell;
    m_cells.get(0, cell);
    s.append(cell.key.to_string());
    s.append(" <= range <= ");
    m_cells.get(-1, cell);
    s.append(cell.key.to_string());
  }

  s.append(" queue=");
  s.append(std::to_string(m_queue.size()));

  s.append(" processing=");
  s.append(std::to_string(m_processing));
  s.append(")");
  return s;
}

const bool Block::_scan(DB::Cells::ReqScan::Ptr req, bool synced) {
  {
    size_t skips = 0; // Ranger::Stats
    std::shared_lock lock(m_mutex);

    m_cells.scan(
      req->spec, 
      req->cells, 
      req->offset,
      [req]() { return req->reached_limits(); },
      skips, 
      req->has_selector 
      ? [req](const DB::Cells::Cell& cell, bool& stop) 
            { return req->selector(cell, stop); }
      : (DB::Cells::Mutable::Selector_t)0 
    );
  }

  processing_decrement();

  if(req->reached_limits()) {
    m_blocks->processing_decrement();
    int err = Error::OK;
    req->response(err);
    return true;
  } else if(!synced) {
    m_blocks->scan(req, ptr());
  }
  return false;
}

void Block::run_queue(int& err) {

  for(DB::Cells::ReqScan::Ptr req; ; ) {
    {
      std::shared_lock lock(m_mutex_state);
      req = m_queue.front();
    }
        
    if(req->type == DB::Cells::ReqScan::Type::BLK_PRELOAD) {
      processing_decrement();

    } else if(err) {
      m_blocks->processing_decrement();
      processing_decrement();
      req->response(err);
          
    } else {
      asio::post(*Env::IoCtx::io()->ptr(), [this, req]() { _scan(req); } );
    }

    {
      std::scoped_lock lock(m_mutex_state);
      m_queue.pop();
      if(m_queue.empty())
        return;
    }
  }
}



}}}
