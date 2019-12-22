/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_RangeBlocks_h
#define swcdb_lib_db_Columns_Rgr_RangeBlocks_h

#include "swcdb/db/Cells/ReqScan.h"

#include "swcdb/db/Files/CellStoreReaders.h"
#include "swcdb/db/Files/CommitLog.h"

namespace SWC { namespace server { namespace Rgr {

class RangeBlocks final {
  public:
  typedef RangeBlocks* Ptr;

  // scan >> blk match >> load-cs + load+logs 

  DB::RangeBase::Ptr           range;
  Files::CommitLog::Fragments  commitlog;
  Files::CellStore::Readers    cellstores;

  class Block : public DB::Cells::RangeBlock {
    public:
    typedef Block* Ptr;

    enum State {
      NONE,
      LOADING,
      LOADED,
      REMOVED
    };
    
    Block::Ptr  next;
    Block::Ptr  prev;

    inline static Ptr make(const DB::Cells::Interval& interval,
                           const RangeBlocks::Ptr& blocks, 
                           State state=State::NONE) {
      return new Block(interval, blocks, state);
    }

    explicit Block(const DB::Cells::Interval& interval, 
                   const RangeBlocks::Ptr& blocks, State state=State::NONE)
                  : DB::Cells::RangeBlock(interval, 
                                     blocks->range->cfg->cell_versions(), 
                                     blocks->range->cfg->cell_ttl(), 
                                     blocks->range->cfg->column_type()), 
                    m_blocks(blocks), 
                    m_state(state), m_processing(0), 
                    next(nullptr), prev(nullptr) {
    }

    Ptr ptr() override {
      return this;
    }

    virtual ~Block() { }

    const bool _is_gt_prev_end(const DB::Cell::Key& key) override {
      return !prev || prev->is_gt_end(key);
    }

    const bool is_consist(const DB::Cells::Interval& intval) override {
      std::shared_lock lock(m_mutex);
      return 
        (intval.key_begin.empty() || m_interval.is_in_end(intval.key_begin))
        && 
        (intval.key_end.empty() || _is_gt_prev_end(intval.key_end));
    }

    const bool splitter() override {
      return m_blocks->_split(ptr(), false);
    }

    const bool removed() {
      std::shared_lock lock(m_mutex_state);
      return m_state == State::REMOVED;
    }

    const bool loaded() {
      std::shared_lock lock(m_mutex_state);
      return m_state == State::LOADED;
    }

    const bool add_logged(const DB::Cells::Cell& cell) {
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
    
    void preload() {
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

    const bool scan(DB::Cells::ReqScan::Ptr req) {
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

      m_blocks->cellstores.load_cells(ptr());
      return true;
    }

    Ptr split(bool loaded) {
      Ptr blk = nullptr;
      if(!m_mutex.try_lock())
        return blk;
      blk = _split(loaded);
      m_mutex.unlock();
      return blk;
    }
    
    Ptr _split(bool loaded) {
      Ptr blk = Block::make(
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

    void add(Ptr blk) {
      //std::scoped_lock lock(m_mutex_state);
      blk->prev = ptr();
      if(next) {
        blk->next = next;
        next->prev = blk;
      }
      next = blk;
      // blk->set_prev_end()
    }

    /*
    void expand_next_and_release(DB::Cell::Key& key_begin) {
      std::scoped_lock lock(m_mutex, m_mutex_state);

      m_state = State::REMOVED;
      key_begin.copy(m_interval.key_begin);
      m_cells.free();
      m_interval.free();
    }

    void merge_and_release(Ptr blk) {
      std::scoped_lock lock(m_mutex, m_mutex_state);

      m_state = State::NONE;
      blk->expand_next_and_release(m_interval.key_begin);
      m_cells.free();
    }
    */

    const size_t release() {
      size_t released = 0;
      if(m_processing || !loaded())
        return released;

      std::scoped_lock lock(m_mutex, m_mutex_state);

      m_state = State::NONE;
      released += m_cells.size();
      m_cells.free();
      return released;
    }

    void processing_increment() {
      m_processing++;
    }

    void processing_decrement() {
      m_processing--;
    }

    const bool processing() const {
      return m_processing;
    }

    const std::string to_string() {
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

    private:

    const bool _scan(DB::Cells::ReqScan::Ptr req, bool synced=false) {
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
    
    void loaded_cellstores(int err) override {
      if(err) {
        SWC_LOGF(LOG_ERROR, "cellstores-load_cells %s err=%d",
                             m_blocks->cellstores.to_string().c_str(), err);
        quick_exit(1); // temporary halt
        run_queue(err);
        return;
      }
      m_blocks->commitlog.load_cells(ptr());
    }

    void loaded_logs(int err) override {
      {
        std::scoped_lock lock(m_mutex_state);
        m_state = State::LOADED;
      }
      if(err) {
        SWC_LOGF(LOG_ERROR, "commitlog-load_cells %s err=%d", 
                             m_blocks->commitlog.to_string().c_str(), err)  
        quick_exit(1); // temporary halt
      }
      run_queue(err);
    }

    void run_queue(int& err) {

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

    std::shared_mutex                    m_mutex_state;
    State                                m_state;
    std::atomic<size_t>                  m_processing;
    std::queue<DB::Cells::ReqScan::Ptr>  m_queue;
    const RangeBlocks::Ptr               m_blocks;
  
  }; // class Block



  explicit RangeBlocks() : m_block(nullptr), m_processing(0) { }
  
  void init(DB::RangeBase::Ptr for_range) {
    range = for_range;
    commitlog.init(range);
    cellstores.init(range);
  }

  Ptr ptr() {
    return this;
  }

  ~RangeBlocks() {  }
  
  void processing_increment() {
    m_processing++;
  }

  void processing_decrement() {
    m_processing--;
  }

  void load(int& err) {
    commitlog.load(err);
    cellstores.load(err);
  }

  void unload() {
    wait_processing();
    commitlog.commit_new_fragment(true);  
    std::scoped_lock lock(m_mutex);
    
    commitlog.unload();
    cellstores.unload();  
    _clear();
    range = nullptr;
  }
  
  void remove(int& err) {
    wait_processing();
    std::scoped_lock lock(m_mutex);

    commitlog.remove(err);
    cellstores.remove(err);   
    _clear();
    range = nullptr;
  }
  
  void apply_new(int &err,
                Files::CellStore::Writers& w_cellstores, 
                std::vector<Files::CommitLog::Fragment::Ptr>& fragments_old) {
    wait_processing();
    std::scoped_lock lock(m_mutex);

    cellstores.replace(err, w_cellstores);
    if(err)
      return;
    Files::RangeData::save(err, cellstores);
      
    commitlog.remove(err, fragments_old);
  }

  const bool need_split(uint32_t sz) const {
    return sz >= range->cfg->block_cells() * 2;
  }

  void add_logged(const DB::Cells::Cell& cell) {
    processing_increment();

    commitlog.add(cell);
    
    bool to_split=false;
    Block::Ptr blk;
    {
      std::shared_lock lock(m_mutex);
      for(blk=m_block; blk; blk=blk->next) {
        if(blk->add_logged(cell)) {
          to_split = blk->loaded();
          break;
        }
      }
    }
    if(to_split)
      split(blk, true);

    processing_decrement();
  }

  void scan(DB::Cells::ReqScan::Ptr req, Block::Ptr blk_ptr = nullptr) {
    if(!blk_ptr)
      processing_increment();

    if(req->expired()) {
      processing_decrement();
      return;
    }    

    int err = Error::OK;
    {
      std::scoped_lock lock(m_mutex);
      if(!m_block) 
        init_blocks(err);
    }
    if(err) {
      processing_decrement();
      req->response(err);
      return;
    }
    for(Block::Ptr eval, nxt_blk, blk=nxt_blk=nullptr; ;
        blk = nullptr, nxt_blk = nullptr) {
      {
        std::shared_lock lock(m_mutex);
        for(eval=blk_ptr? blk_ptr->next : m_block; eval; eval=eval->next) {
          if(eval->removed() || !eval->is_next(req->spec)) 
            continue;
          blk = eval;
          blk_ptr = blk;
          blk->processing_increment();

          if((!req->spec.flags.limit || req->spec.flags.limit > 1) 
              && eval->next 
              && !Env::Resources.need_ram(range->cfg->block_size() * 10)
              && !eval->next->loaded() && !eval->next->removed()) {
            nxt_blk = eval->next;
            nxt_blk->processing_increment();
          }
          break;
        }
      }
      if(blk == nullptr)
        break;
      
      if(nxt_blk != nullptr) {
        if(nxt_blk->includes(req->spec))
          nxt_blk->preload();
        else 
          nxt_blk->processing_decrement();
      }

      if(Env::Resources.need_ram(range->cfg->block_size())) {
        asio::post(*Env::IoCtx::io()->ptr(), 
          [blk, ptr=ptr()](){
            ptr->release_prior(blk); // release_and_merge(blk);
          }
        );
      }

      if(blk->scan(req)) // true (queued || responded)
        return;
    }

    processing_decrement();

    req->response(err);
  }

  void split(Block::Ptr blk, bool loaded=true) {
    if(need_split(blk->size()) && m_mutex.try_lock()) {
      do blk = blk->split(loaded);
      while(need_split(blk->size()));
      m_mutex.unlock();
    }
  }

  const bool _split(Block::Ptr blk, bool loaded=true) {
    // blk is under lock
    if(need_split(blk->_size()) && m_mutex.try_lock()) {
      do blk = blk->_split(loaded);
      while(need_split(blk->_size()));
      m_mutex.unlock();
      return true;
    }
    return false;
  }

  const size_t cells_count() {
    size_t sz = 0;
    std::shared_lock lock(m_mutex);
    for(Block::Ptr blk=m_block; blk; blk=blk->next)
      sz += blk->size();
    return sz;
  }

  const size_t size() {
    std::shared_lock lock(m_mutex);
    return _size();
  }

  const size_t size_bytes() {
    std::shared_lock lock(m_mutex);
    return _size_bytes();
  }

  const size_t size_bytes_total(bool only_loaded=false) {
    std::shared_lock lock(m_mutex);
    return _size_bytes() 
          + cellstores.size_bytes(only_loaded) 
          + commitlog.size_bytes(only_loaded);
  }

  void release_prior(Block::Ptr ptr) {
    std::shared_lock lock(m_mutex);
    if(ptr->prev)
      ptr->prev->release();
  }

  /*
  void release_and_merge(Block::Ptr ptr) {
    std::scoped_lock lock(m_mutex);
    bool state = false;
    for(size_t idx = 0; idx<m_blocks.size(); idx++) {
      if(ptr == m_blocks[idx]) {
        if(idx < 2)
          return;
        auto ptr1 = m_blocks[idx-2];
        if(ptr1->processing())
          return;
        auto ptr2 = m_blocks[idx-1];
        if(ptr2->processing())
          return;
        ptr2->merge_and_release(ptr1);
        delete ptr1;
        m_blocks.erase(m_blocks.begin()+idx-2);
        --idx;
        //state = true;
      }
    }
    if(state) {
      for(auto blk : m_blocks)
        std::cout << " " << blk->to_string() << "\n"; 
    }
  }
  */

  const size_t release(size_t bytes=0) {
    size_t released = cellstores.release(bytes);
    if(!bytes || released < bytes) {

      released += commitlog.release(bytes ? bytes-released : bytes);
      if(!bytes || released < bytes) {

        std::shared_lock lock(m_mutex);
        for(Block::Ptr blk=m_block; blk; blk=blk->next) {
          released += blk->release();
          if(bytes && released >= bytes)
            break;
        }
      }
    }
    if(!bytes && !processing()) {
      std::scoped_lock lock(m_mutex);
      _clear();
      bytes = 0;
    }
    //else if(_size() > 1000)
    // merge in pairs down to 1000 blks
    
    return released;
  }

  const bool processing() {
    std::shared_lock lock(m_mutex);
    return _processing();
  }

  void wait_processing() {
    while(processing() || commitlog.processing() || cellstores.processing())
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  const std::string to_string(){
    std::scoped_lock lock(m_mutex);

    std::string s("RangeBlocks(count=");
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

  private:

  const size_t _size() {
    size_t sz = 0;
    for(Block::Ptr blk=m_block; blk; blk=blk->next)
      sz++;
    return sz;
  }

  const size_t _size_bytes() {
    size_t sz = 0;
    for(Block::Ptr blk=m_block; blk; blk=blk->next)
      sz += blk->size_bytes();
    return sz;
  }
  
  const bool _processing() const {
    if(m_processing)
      return true;
    for(Block::Ptr blk=m_block; blk; blk=blk->next) {
      if(blk->processing()) 
        return true;
    }
    return false;
  }

  void _clear() {
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

  void init_blocks(int& err) {
    std::vector<Files::CellStore::Block::Read::Ptr> blocks;
    cellstores.get_blocks(err, blocks);
    if(err) {
      _clear();
      return;
    }

    Block::Ptr blk = nullptr;
    Block::Ptr new_blk;
    for(auto cs_blk : blocks) {
      new_blk = Block::make(cs_blk->interval, ptr());
      if(blk == nullptr)
        m_block = new_blk;
      else
        blk->add(new_blk);
      blk = new_blk;
    }
    if(!m_block) {
      err = Error::RS_NOT_LOADED_RANGE;
      return;
    }

    if(range->is_any_begin())
      m_block->free_key_begin();
    if(range->is_any_end()) 
      blk->free_key_end();

  }

  std::shared_mutex   m_mutex;
  Block::Ptr          m_block;
  std::atomic<size_t> m_processing;

};





}}}
#endif