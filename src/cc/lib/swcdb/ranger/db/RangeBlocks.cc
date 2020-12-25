/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlocks.h"

namespace SWC { namespace Ranger {


Blocks::Blocks(const DB::Types::KeySeq key_seq) 
              : commitlog(key_seq), 
                m_block(nullptr), m_processing(0) { 
}
  
void Blocks::init(const RangePtr& for_range) {
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

  Core::MutexSptd::scope lock(m_mutex);
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    blk->schema_update();
}

SWC_SHOULD_INLINE
void Blocks::processing_increment() {
  m_processing.fetch_add(1);
}

SWC_SHOULD_INLINE
void Blocks::processing_decrement() {
  m_processing.fetch_sub(1);
}

void Blocks::load(int& err) {
  commitlog.load(err);
  cellstores.load(err);
}

void Blocks::unload() {
  wait_processing();
  processing_increment();
  commitlog.commit_new_fragment(true);  
  Core::MutexSptd::scope lock(m_mutex);
    
  commitlog.unload();
  cellstores.unload();  
  _clear();
  range = nullptr;
  processing_decrement();
}
  
void Blocks::remove(int&) {
  wait_processing();
  processing_increment();
  Core::MutexSptd::scope lock(m_mutex);

  // Unload is enough, Range remove the range-path at once
  commitlog.remove();
  cellstores.unload();

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
  Core::MutexSptd::scope lock(m_mutex);

  cellstores.replace(err, w_cellstores);
  if(err)
    return;
  RangeData::save(err, cellstores);
      
  commitlog.remove(err, fragments_old);
}

void Blocks::add_logged(const DB::Cells::Cell& cell) {

  commitlog.add(cell);

  Block::Ptr blk;
  {
    Core::MutexSptd::scope lock(m_mutex);
    blk = m_block ? *(m_blocks_idx.begin()+_narrow(cell.key)) : nullptr;
  }
  while(blk && !blk->add_logged(cell)) {
    Core::MutexSptd::scope lock(m_mutex);
    blk = blk->next;
  }
}

void Blocks::scan(ReqScan::Ptr req, Block::Ptr blk_ptr) {
  if(!blk_ptr)
    processing_increment();

  if(req->expired()) {
    processing_decrement();
    return;
  }    

  int err = Error::OK;
  range->state(err);
  if(!err) {
    Core::MutexSptd::scope lock(m_mutex);
    if(!m_block)
      init_blocks(err);
  }

  if(err) {
    processing_decrement();
    req->response(err);
    return;
  }

  const size_t need = range->cfg->block_size() * 4;

  for(Block::Ptr blk = nullptr; ; blk = nullptr) {
    int64_t ts = Time::now_ns();

    if(req->with_block() && req->block) {
      (blk = blk_ptr = (Block*)req->block)->processing_increment();
      req->block = nullptr;

    } else {
      if(blk_ptr && Env::Rgr::res().need_ram(need * (req->readahead + 1)))
        blk_ptr->release();

      bool support = m_mutex.lock();
      blk_ptr = blk_ptr 
        ? blk_ptr->next 
        : (req->spec.offset_key.empty()
            ? m_block 
            : *(m_blocks_idx.begin() + _narrow(req->spec.offset_key)));
      m_mutex.unlock(support);

      while(blk_ptr && !blk_ptr->is_next(req->spec)) {
        support = m_mutex.lock();
        blk_ptr = blk_ptr->next;
        m_mutex.unlock(support);
      }

      if(blk_ptr) {
        (blk = blk_ptr)->processing_increment();

        if(Env::Rgr::res().need_ram(need * (req->readahead + 1))) {
          size_t sz;
          size_t _need = need * (req->readahead + 1);
          for(Block::Ptr prev = blk; ; _need -= sz ) {
            support = m_mutex.lock();
            prev = prev->prev;
            m_mutex.unlock(support);
            if(!prev || (sz = prev->release()) > _need)
              break;
          }
        }
      }
    }

    req->profile.add_block_locate(ts);
    if(!blk)
      break;

    switch(blk->scan(req)) {

      case Block::ScanState::RESPONDED:
        return;

      case Block::ScanState::QUEUED: {
        processing_increment();
        bool support;
        for(size_t n=0; 
            n < req->readahead && 
            !Env::Rgr::res().need_ram(need * (++n))
            && m_mutex.try_full_lock(support); ) {
          blk = blk->next;
          m_mutex.unlock(support);
          if(!blk || commitlog.is_compacting())
            break;
          if(blk->need_load() && blk->includes(req->spec)) {
            blk->processing_increment();
            blk->preload();
          }
        }
        processing_decrement();
        return;
      }

      default:
        break;
    }

  }

  processing_decrement();

  req->response(err);
}


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
        if((preload = !commitlog.is_compacting() &&
                  range->is_loaded() && range->compacting() &&
                  !Env::Rgr::res().need_ram(range->cfg->block_size() * 10)))
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
  Core::MutexSptd::scope lock(m_mutex);
  for(Block::Ptr blk=m_block; blk; blk=blk->next)
    sz += blk->size();
  return sz;
}

size_t Blocks::size() {
  Core::MutexSptd::scope lock(m_mutex);
  return _size();
}

size_t Blocks::size_bytes() {
  Core::MutexSptd::scope lock(m_mutex);
  return _size_bytes();
}

size_t Blocks::size_bytes_total(bool only_loaded) {
  Core::MutexSptd::scope lock(m_mutex);
  return _size_bytes() 
        + cellstores.size_bytes(only_loaded) 
        + commitlog.size_bytes(only_loaded);  
}

size_t Blocks::release(size_t bytes) {
  size_t released = cellstores.release(bytes);
  if(!bytes || released < bytes) {

    released += commitlog.release(bytes ? bytes-released : bytes);
    if(!bytes || released < bytes) {
      bool support;
      bool ok;
      if(bytes) {
        ok = m_mutex.try_full_lock(support);
      } else {
        support = m_mutex.lock();
        ok = true;
      }
      if(ok) {
        for(Block::Ptr blk=m_block; blk; blk=blk->next) {
          released += blk->release();
          if(bytes && released >= bytes)
            break;
        }
        m_mutex.unlock(support);
      }
    }
  }
  if(!bytes && !processing()) {
    Core::MutexSptd::scope lock(m_mutex);
    _clear();
    bytes = 0;
  }
  //else if(_size() > 1000)
  // merge in pairs down to 1000 blks
    
  return released;
}

bool Blocks::processing() noexcept {
  bool support;
  bool busy;
  if(!(busy = !m_mutex.try_full_lock(support))) {
    busy = _processing();
    m_mutex.unlock(support);
  }
  return busy;
}

void Blocks::wait_processing() {
  do {
    while(processing() || commitlog.processing() || cellstores.processing())
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    std::this_thread::yield();
  } while(processing() || commitlog.processing() || cellstores.processing());
}

void Blocks::print(std::ostream& out, bool minimal) {
  Core::MutexSptd::scope lock(m_mutex);
  out << "Blocks(count=" << _size();
  if(minimal) {
    if(m_block) {
      Block::Ptr blk = m_block;
      blk->print(out << " first=");
      for(; blk->next; blk=blk->next);
      if(blk != m_block) {
        blk->print(out << " last=");
      }
    }
  } else {
    out << " blocks=[";
    for(Block::Ptr blk = m_block; blk; blk=blk->next) {
      blk->print(out); 
      out << ", ";
    }
    out << ']';
  }
  commitlog.print(out << ' ', minimal);
  cellstores.print(out << ' ', minimal);
  out 
    << " processing=" << _processing()
    << " bytes=" << _size_bytes() 
    << ')';
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
  
bool Blocks::_processing() const noexcept {
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

  Block::Ptr blk = nullptr;
  for(auto cs_blk : blocks) {
    if(!blk) {
      m_block = blk = Block::make(cs_blk->header.interval, ptr());
      m_block->set_prev_key_end(range->prev_range_end);
      m_blocks_idx.push_back(blk);
    } else if(blk->cond_key_end(cs_blk->header.interval.key_begin) 
                                            != Condition::EQ) {
      blk->_add(Block::make(cs_blk->header.interval, ptr()));
      blk = blk->next;
      m_blocks_idx.push_back(blk);
    } else {
      blk->set_key_end(cs_blk->header.interval.key_end);
    }
  }
  if(!m_block) {
    err = Error::RGR_NOT_LOADED_RANGE;
    return;
  }

  if(range->_is_any_end()) 
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
  if(key.empty() || m_blocks_idx.size() <= MAX_IDX_NARROW)
    return offset;
  
  size_t step = offset = m_blocks_idx.size() >> 1;
  try_narrow:
    if(!(*(m_blocks_idx.begin() + offset))->is_in_end(key)) {
      if(step < MAX_IDX_NARROW)
        return offset;
      offset += step >>= 1;
      goto try_narrow;
    }
    if((step >>= 1) <= MAX_IDX_NARROW)
      ++step;
    if(offset < step)
      return 0;
    offset -= step;
  goto try_narrow;
}


}}