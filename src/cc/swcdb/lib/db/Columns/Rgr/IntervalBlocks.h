/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_IntervalBlocks_h
#define swcdb_lib_db_Columns_Rgr_IntervalBlocks_h

#include "swcdb/lib/db/Files/CellStoreReaders.h"
#include "swcdb/lib/db/Files/CommitLog.h"

namespace SWC { namespace server { namespace Rgr {

class IntervalBlocks {
  public:
  typedef IntervalBlocks* Ptr;


  DB::RangeBase::Ptr                range;
  Files::CommitLog::Fragments::Ptr  commitlog;
  Files::CellStore::Readers::Ptr    cellstores;


  class Block {
    public:
    typedef Block* Ptr;

    enum State {
      NONE,
      LOADING,
      LOADED
    };

    Files::CellsBlock::Ptr    cells_block;   
    IntervalBlocks::Ptr       blocks;
    std::atomic<size_t>       processing;
    
    inline static Ptr make(Files::CellsBlock::Ptr cells_block, 
                           IntervalBlocks::Ptr blocks,
                           State state=State::NONE) {
      return new Block(cells_block, blocks, state);
    }

    Block(Files::CellsBlock::Ptr cells_block, IntervalBlocks::Ptr blocks,
          State state=State::NONE)
          : cells_block(cells_block), blocks(blocks), m_state(state), 
            processing(0) {
    }
    
    Ptr ptr() {
      return this;
    }

    virtual ~Block() {
      std::cout << " IntervalBlocks::~Block\n";
      wait_processing();
      delete cells_block;
    }

    bool loaded() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_state = Block::State::LOADED;
    }
    
    size_t release() {
      return 0;
    }

    bool add_logged(const DB::Cells::Cell& cell) { 

      if(!cells_block->interval.is_in_end(cell.key))
        return false;

      cells_block->cells.add(cell);

      if(cell.on_fraction)
        // return added for fraction under current end
        return cells_block->interval.key_end.compare(
          cell.key, cell.on_fraction) != Condition::GT;
      
      if(!cells_block->interval.is_in_begin(cell.key)) {
        cells_block->interval.key_begin.copy(cell.key); 
        cells_block->interval.expand(cell.timestamp);
      }
      return true;
    }
    
    bool scan(DB::Cells::ReqScan::Ptr req) {
      //std::cout << " Block::scan "<< to_string()<<" \n";
      State state;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        state = m_state;
      }

      if(state == Block::State::LOADED) {
        _scan(req);
        return false;
      } 
      
      bool run;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_queue.push(new Callback(req, ptr()));

        if((run = state == Block::State::NONE))
          m_state = Block::State::LOADING;
      }

      if(run) {
        bool expected = blocks->cellstores->foreach(
          cells_block->interval, 
          [ptr=ptr()](const Files::CellStore::Read::Ptr& cs){
            cs->load_cells(
              ptr->cells_block, 
              [ptr](int err) {
                ptr->loaded_cellstores(err);
              }
            );
          }
        );
        if(!expected) {
          int err = Error::OK;
          loaded_cellstores(err);
        }
      }
      return true;
    }

    void loaded_cellstores(int& err) {
      if(err) {
        run_queue(err);
        return;
      }
      
      blocks->commitlog->load_cells(
        cells_block,
        [ptr=ptr()](int err){
          ptr->loaded_logs(err);
        }
      );
    }

    void loaded_logs(int& err) {
      blocks->split(ptr());
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = Block::State::LOADED;
      }
      run_queue(err);
    }

    void run_queue(int& err) {
      Callback* cb;
    
      for(;;) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          if(m_queue.empty())
            return;
          cb = m_queue.front();
        }

        cb->call(err);
        delete cb;
      
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_queue.pop();
        }
      }
    }

    Ptr split(size_t keep=100000) {
      std::lock_guard<std::mutex> lock(m_mutex);

      Ptr blk = Block::make(
        Files::CellsBlock::make(
          DB::Cells::Interval(), 
          Env::Schemas::get()->get(blocks->range->cid)
        ), 
        blocks,
        Block::State::LOADED
      );

      cells_block->cells.move_from_key_offset(
        keep, blk->cells_block->cells);
      
      bool any_begin = cells_block->interval.key_begin.empty();
      bool any_end = cells_block->interval.key_end.empty();
      cells_block->interval.free();

      cells_block->cells.expand(cells_block->interval);
      blk->cells_block->cells.expand(blk->cells_block->interval);

      if(any_begin)
        cells_block->interval.key_begin.free();
      if(any_end)
        blk->cells_block->interval.key_end.free();
      return blk;
    }

    void wait_processing() {
      while(processing > 0) {
        std::cout << "wait_processing: " << to_string() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }

    const std::string to_string() {
      std::lock_guard<std::mutex> lock(m_mutex);

      std::string s("Block(state=");
      s.append(std::to_string((uint8_t)m_state));
      
      s.append(" ");
      s.append(cells_block->to_string());

      s.append(" queue=");
      s.append(std::to_string(m_queue.size()));

      s.append(" processing=");
      s.append(std::to_string(processing.load()));
      s.append(")");
      return s;
    }

    private:

    void _scan(DB::Cells::ReqScan::Ptr req) {
      size_t skips = 0; // Ranger::Stats
      cells_block->cells.scan(
        *(req->spec).get(), 
        req->cells, 
        req->offset,
        [req]() { return req->reached_limits(); },
        skips, 
        req->has_selector 
        ? [req](const DB::Cells::Cell& cell) 
              { return req->selector(cell); }
        : (DB::Cells::Mutable::Selector_t)0 
      );  

      //if(req->cells->size())
      //  req->cells->get(
      //    -1, req->spec->offset_key, req->spec->offset_rev);
    }
    
    struct Callback {
      public:
      DB::Cells::ReqScan::Ptr req;
      Block::Ptr              ptr;

      Callback(DB::Cells::ReqScan::Ptr req, Block::Ptr ptr) 
               : req(req), ptr(ptr) { }

      virtual ~Callback() { }

      void call(int err) {
        if(err) {
          req->response(err);
          return;
        }

        ptr->_scan(req); 
        
        if(req->reached_limits()) {
          req->response(err);
        } else {
          ptr->blocks->scan(req, ptr); 
        }
      }
    };
    
    std::mutex             m_mutex;
    State                  m_state;
    std::queue<Callback*>  m_queue;

  }; // class Block


  // scan >> blk match >> load-cs + load+logs 

  IntervalBlocks(): commitlog(nullptr), cellstores(nullptr) {
  }
  
  void init(DB::RangeBase::Ptr for_range) {
    range       = for_range;
    commitlog   = Files::CommitLog::Fragments::make(range);
    cellstores  = Files::CellStore::Readers::make(range);
  }

  Ptr ptr() {
    return this;
  }

  virtual ~IntervalBlocks(){
    //std::cout << " ~IntervalBlocks\n";
    free();
  }
  
  void load(int& err) {
    commitlog->load(err);
  }

  void unload() {
    stop(Error::RS_NOT_READY);

    std::lock_guard<std::mutex> lock(m_mutex);

    if(commitlog != nullptr) 
      commitlog->commit_new_fragment(true);

    free();
  }
  
  void remove(int& err) {
    stop(Error::COLUMN_MARKED_REMOVED);

    std::lock_guard<std::mutex> lock(m_mutex);

    if(commitlog != nullptr) 
      commitlog->remove(err);
    if(cellstores != nullptr)
      cellstores->remove(err);  

    free();
  }

  void stop(int err) {
    std::vector<Block::Ptr>::iterator it;
    bool started=false;
    for(;;){
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!started) {
          it = m_blocks.begin();
          started = true;
        } else 
          it++;
        if(it == m_blocks.end())
          break;
      }
      (*it)->run_queue(err);
    }
    wait_processing();
  }
  
  void add_logged(const DB::Cells::Cell& cell) {
    commitlog->add(cell);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for(size_t idx = 0; idx<m_blocks.size(); idx++) {
      if(m_blocks[idx]->loaded())
        split(idx);

      if(m_blocks[idx]->add_logged(cell))
        break;
    }
    
  }

  void scan(DB::Cells::ReqScan::Ptr req, Block::Ptr blk_ptr = nullptr) {
    //std::cout << "IntervalBlocks::scan " <<  to_string() 
    //          << " " << req->to_string() << "\n";

    int err = Error::OK;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(cellstores->empty())
        err = Error::RS_NOT_LOADED_RANGE;

      else if(m_blocks.empty()) 
        init_blocks(err);
    }
    if(err) {
      req->response(err);
      return;
    }

    
    Block::Ptr blk;
    bool flw;
    for(;;) {
      {
        blk = nullptr;
        flw = false;
        std::lock_guard<std::mutex> lock(m_mutex);
        // it = narrower on req->spec->offset_key

        for(auto& eval : m_blocks) {
          if(!flw && blk_ptr != nullptr) {
            if(blk_ptr == eval)
              flw = true;
            continue;
          }
          if((req->spec->offset_key.empty() 
              || eval->cells_block->interval.is_in_end(req->spec->offset_key))
             && eval->cells_block->interval.includes(req->spec)) {
            blk = eval;
            blk_ptr = eval;
            break;
          }
        }
        if(blk == nullptr)  
          break;
      }

      if(blk->scan(req))
        return;
      
      if(req->reached_limits())
        break;
    }

    req->response(err);
  }

  const size_t cells_count() {
    size_t sz = 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& blk : m_blocks) {
      sz += blk->cells_block->cells.size();
    }
    return sz;
  }
  
  void split() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for(size_t idx = 0; idx<m_blocks.size(); idx++) {
      if(m_blocks[idx]->loaded())
        split(idx);
    }
  }

  void split(Block* ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for(size_t idx = 0; idx<m_blocks.size(); idx++) {
      if(ptr == m_blocks[idx]) {
        split(idx);
        return;
      }
    }
  }

  const size_t release(size_t bytes=0) {
    size_t released = cellstores->release(bytes);
    if(bytes && released >= bytes)
      return released;
    
    released += commitlog->release(bytes);
    if(bytes && released >= bytes)
      return released;

    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto blk : m_blocks) {
      released += blk->release();
      if(bytes && released >= bytes)
        return released;
    }

    return released;
  }

  const size_t processing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _processing();
  }

  void wait_processing() {
    while(processing() > 0) {
      std::cout << "wait_processing: " << to_string() << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("IntervalBlocks(count=");
    s.append(std::to_string(m_blocks.size()));

    s.append(" blocks=[");
    for(auto blk : m_blocks) {
        s.append(blk->to_string());
       s.append(", ");
    }
    s.append("] ");

    if(commitlog != nullptr)
      s.append(commitlog->to_string());

    s.append(" ");
    if(cellstores != nullptr)
      s.append(cellstores->to_string());

    s.append(" processing=");
    s.append(std::to_string(_processing()));

    s.append(")");
    return s;
  } 

  private:
  
  const size_t _processing() {
    size_t sz = 0;
    for(auto blk : m_blocks) 
      sz += blk->processing;
    return sz;
  }

  void free() {
    if(cellstores != nullptr) {
      delete cellstores;
      cellstores = nullptr;
    }
    if(commitlog != nullptr) {
      delete commitlog;
      commitlog = nullptr;
    }
    range = nullptr;
    
    for(auto & blk : m_blocks) 
      delete blk;
    m_blocks.clear();
  }

  void init_blocks(int& err) {
    std::vector<Files::CellStore::Block::Read::Ptr> blocks;
    cellstores->get_blocks(err, blocks);
    if(err) {
      for(auto& blk : m_blocks) 
        delete blk;
      m_blocks.clear();
      return;
    }

    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
    for(auto& blk : blocks) {
      m_blocks.push_back(
        Block::make(
          Files::CellsBlock::make(blk->interval, schema), 
          ptr()
        )
      );
    }
    if(range->is_any_begin())
      m_blocks.front()->cells_block->interval.key_begin.free();
    if(range->is_any_end()) 
      m_blocks.back()->cells_block->interval.key_end.free();
  }

  void split(size_t idx) {
    while(m_blocks[idx]->cells_block->cells.size() >= 200000) {
      auto blk = m_blocks[idx++]->split(100000);
      m_blocks.insert(m_blocks.begin()+idx, blk);
    }
  }

  std::mutex                   m_mutex;
  std::vector<Block::Ptr>      m_blocks;

};





}}}
#endif