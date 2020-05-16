/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/RangeBlocks.h"

namespace SWC { namespace Ranger {


Blocks::Blocks(const Types::KeySeq key_seq) 
              : commitlog(key_seq), 
                m_block(nullptr), m_processing(0) { 
}
  
void Blocks::init(RangePtr for_range) {
  range = for_range;
  commitlog.init(range);
  cellstores.init(range);
}

SWC_SHOULD_INLINE
Blocks::Ptr Blocks::ptr() {
  return this;
}

Blocks::~Blocks() {  }

void Blocks::schema_update() {
  commitlog.schema_update();

  Mutex::scope lock(m_mutex);
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    blk->schema_update();
}

SWC_SHOULD_INLINE
void Blocks::processing_increment() {
  ++m_processing;
}

SWC_SHOULD_INLINE
void Blocks::processing_decrement() {
  --m_processing;
}

void Blocks::load(int& err) {
  commitlog.load(err);
  cellstores.load(err);
}

void Blocks::unload() {
  wait_processing();
  processing_increment();
  commitlog.commit_new_fragment(true);  
  Mutex::scope lock(m_mutex);
    
  commitlog.unload();
  cellstores.unload();  
  _clear();
  range = nullptr;
  processing_decrement();
}
  
void Blocks::remove(int& err) {
  wait_processing();
  processing_increment();
  Mutex::scope lock(m_mutex);

  commitlog.remove(err);
  cellstores.remove(err);   
  _clear();
  range = nullptr;
  processing_decrement();
}

void Blocks::expand(DB::Cells::Interval& intval) {
  cellstores.expand(intval);
  commitlog.expand(intval);
}

void Blocks::expand_and_align(DB::Cells::Interval& intval) {
  cellstores.expand_and_align(intval);
  commitlog.expand_and_align(intval);
}

void Blocks::apply_new(int &err,
                       CellStore::Writers& w_cellstores, 
                       CommitLog::Fragments::Vec& fragments_old) {
  wait_processing();
  Mutex::scope lock(m_mutex);

  cellstores.replace(err, w_cellstores);
  if(err)
    return;
  RangeData::save(err, cellstores);
      
  commitlog.remove(err, fragments_old);
}

void Blocks::add_logged(const DB::Cells::Cell& cell) {
  processing_increment();

  commitlog.add(cell);
    
  Block::Ptr blk;
  {
    Mutex::scope lock(m_mutex);
    blk = m_block ? *(m_blocks_idx.begin()+_narrow(cell.key)) : nullptr;
  }
  while(blk && !blk->add_logged(cell)) {
    Mutex::scope lock(m_mutex);
    blk = blk->next;
  }

  processing_decrement();
}

void Blocks::scan(ReqScan::Ptr req, Block::Ptr blk_ptr) {
  if(!blk_ptr)
    processing_increment();

  if(req->expired()) {
    processing_decrement();
    return;
  }    

  int err = Error::OK;
  {
    Mutex::scope lock(m_mutex);
    if(!m_block) 
      init_blocks(err);
  }
  if(err) {
    processing_decrement();
    req->response(err);
    return;
  }

  std::vector<Block::Ptr> nxt_blks;

  for(Block::Ptr eval, blk=nullptr; ; blk = nullptr) {
    if(req->with_block() && req->block) {
      (blk = blk_ptr = (Block*)req->block)->processing_increment();
      req->block = nullptr;

    } else {
      {
        Mutex::scope lock(m_mutex);
        eval = blk_ptr ? blk_ptr->next 
               : (req->spec.offset_key.empty() 
                  ? m_block 
                  : *(m_blocks_idx.begin()+_narrow(req->spec.offset_key)));
        for(; eval; eval=eval->next) {
          if(eval->removed() || !eval->is_next(req->spec)) 
            continue;
          (blk = blk_ptr = eval)->processing_increment();

          for(uint8_t n=1; eval->next && n <= req->readahead; ++n) {
            if(Env::Resources.need_ram(range->cfg->block_size() * 10 * n))
              break;
            if((eval = eval->next)->loaded()) 
              continue;
            nxt_blks.push_back(eval); 
            eval->processing_increment();
          }
          break;
        }
      }

      if(!blk)
        break;

      if(!nxt_blks.empty()) {
        asio::post(*Env::IoCtx::io()->ptr(), 
          [this, req, nxt_blks]() { preload(req, nxt_blks); } );
        nxt_blks.clear();
      }

      if(Env::Resources.need_ram(range->cfg->block_size() * 3))
        release_prior(blk); // release_and_merge(blk);
    }

    if(blk->scan(req)) // true (queued || responded)
      return;
  }

  processing_decrement();

  req->response(err);
}

void Blocks::preload(ReqScan::Ptr req, const std::vector<Block::Ptr>& blks) {
  for(auto nxt_blk : blks) {
    if(nxt_blk->includes(req->spec))
      nxt_blk->preload();
    else
      nxt_blk->processing_decrement();
  }
}

/*
void Blocks::split(Block::Ptr blk, bool loaded) {
  bool support;
  if(blk->need_split() && m_mutex.try_full_lock(support) {
    auto offset = _get_block_idx(blk);
    do {
      if((blk = blk->split(loaded)) == nullptr)
        break;
      m_blocks_idx.insert(m_blocks_idx.begin()+(++offset), blk);
    } while(blk->need_split());
    m_mutex.unlock(support);
  }
}
*/

bool Blocks::_split(Block::Ptr blk, bool loaded) {
  // call is under blk lock
  bool support;
  if(blk->_need_split() && m_mutex.try_full_lock(support)) {
   bool preload = false;
    auto offset = _get_block_idx(blk);
    do {
      blk = blk->_split(loaded);
      m_blocks_idx.insert(m_blocks_idx.begin()+(++offset), blk);
      if(!blk->loaded()) {
        if(preload = range->is_loaded() && range->compacting() &&
                     !Env::Resources.need_ram(range->cfg->block_size() * 10))
          blk->processing_increment();
        break;
      }
    } while(blk->_need_split());
    m_mutex.unlock(support);

    if(preload)
      blk->preload();
    return true;
  }
  return false;
}

size_t Blocks::cells_count() {
  size_t sz = 0;
  Mutex::scope lock(m_mutex);
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    sz += blk->size();
  return sz;
}

size_t Blocks::size() {
  Mutex::scope lock(m_mutex);
  return _size();
}

size_t Blocks::size_bytes() {
  Mutex::scope lock(m_mutex);
  return _size_bytes();
}

size_t Blocks::size_bytes_total(bool only_loaded) {
  Mutex::scope lock(m_mutex);
  return _size_bytes() 
        + cellstores.size_bytes(only_loaded) 
        + commitlog.size_bytes(only_loaded);  
}

void Blocks::release_prior(Block::Ptr blk) {
  bool support;
  if(m_mutex.try_full_lock(support)) {
    blk = blk->prev;
    m_mutex.unlock(support);
  }
  if(blk)
    blk->release();
}

size_t Blocks::release(size_t bytes) {
  size_t released = cellstores.release(bytes);
  if(!bytes || released < bytes) {

    released += commitlog.release(bytes ? bytes-released : bytes);
    if(!bytes || released < bytes) {

      Mutex::scope lock(m_mutex);
      for(Block::Ptr blk=m_block; blk; blk=blk->next) {
        released += blk->release();
        if(bytes && released >= bytes)
          break;
      }
    }
  }
  if(!bytes && !processing()) {
    Mutex::scope lock(m_mutex);
    _clear();
    bytes = 0;
  }
  //else if(_size() > 1000)
  // merge in pairs down to 1000 blks
    
  return released;
}

bool Blocks::processing() {
  Mutex::scope lock(m_mutex);
  return _processing();
}

void Blocks::wait_processing() {
  while(processing() || commitlog.processing() || cellstores.processing())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

std::string Blocks::to_string() {
  Mutex::scope lock(m_mutex);

  std::string s("Blocks(count=");
  s.append(std::to_string(_size()));

  s.append(" blocks=[");
    
  for(Block::Ptr blk = m_block; blk; blk=blk->next) {
    s.append(blk->to_string());
    s.append(", ");
  }
  s.append("] ");

  s.append(commitlog.to_string());

  s.append(" ");
  s.append(cellstores.to_string());

  s.append(" processing=");
  s.append(std::to_string(_processing()));

  s.append(" bytes=");
  s.append(std::to_string(_size_bytes()));

  s.append(")");
  return s;
} 

size_t Blocks::_size() {
  size_t sz = 0;
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    ++sz;
  return sz;
}

size_t Blocks::_size_bytes() {
  size_t sz = 0;
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    sz += blk->size_bytes();
  return sz;
}
  
bool Blocks::_processing() const {
  if(m_processing)
    return true;
  for(Block::Ptr blk=m_block; blk; blk=blk->next) {
    if(blk->processing()) 
      return true;
  }
  return false;
}

void Blocks::_clear() {
  m_blocks_idx.clear();
  Block::Ptr blk = m_block;
  for(; blk; blk=blk->next) {
    if(blk->prev)
      delete blk->prev;
    if(!blk->next) {
      delete blk;
      break;
    }
  }
  m_block = nullptr;
}

void Blocks::init_blocks(int& err) {
  std::vector<CellStore::Block::Read::Ptr> blocks;
  cellstores.get_blocks(err, blocks);
  if(err) {
    _clear();
    return;
  }

  DB::Cell::Key prev_key_end;
  if(!range->is_any_begin()) {
    cellstores.get_prev_key_end(0, prev_key_end);
    range->set_prev_key_end(prev_key_end);
  }

  Block::Ptr blk = nullptr;
  for(auto cs_blk : blocks) {
    if(blk == nullptr) {
      m_block = blk = Block::make(cs_blk->interval, ptr());
      m_block->_set_prev_key_end(prev_key_end);
      m_blocks_idx.push_back(blk);
    } else if(blk->_cond_key_end(cs_blk->interval.key_begin) 
                                            != Condition::EQ) {
      blk->_add(Block::make(cs_blk->interval, ptr()));
      blk = blk->next;
      m_blocks_idx.push_back(blk);
    } else {
      blk->_set_key_end(cs_blk->interval.key_end);
    }
  }
  if(!m_block) {
    err = Error::RS_NOT_LOADED_RANGE;
    return;
  }

  if(range->is_any_end()) 
    blk->free_key_end();
}

size_t Blocks::_get_block_idx(Block::Ptr blk) const {
  for(auto it=m_blocks_idx.begin(); it < m_blocks_idx.end();++it)
    if(*it == blk)
      return it - m_blocks_idx.begin();
  return 0; // eq m_block;
}

size_t Blocks::_narrow(const DB::Cell::Key& key) const {
    size_t offset = 0;
    if(m_blocks_idx.size() < MAX_IDX_NARROW || key.empty())
      return offset;
      
    for(size_t sz = offset = m_blocks_idx.size() >> 1;;) {
      if(!(*(m_blocks_idx.begin() + offset))->is_in_end(key)) {
        if(sz < MAX_IDX_NARROW)
          break;
        offset += sz >>= 1; 
        continue;
      }
      if((sz >>= 1) == 0)
        ++sz;  
      if(offset < sz) {
        offset = 0;
        break;
      }
      offset -= sz;
    }
    return offset;
}


}}