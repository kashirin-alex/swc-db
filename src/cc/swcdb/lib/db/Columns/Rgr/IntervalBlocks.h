/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_IntervalBlocks_h
#define swcdb_lib_db_Columns_Rgr_IntervalBlocks_h


namespace SWC { namespace server { namespace Rgr {

class IntervalBlocks : public std::enable_shared_from_this<IntervalBlocks> {
  public:
  typedef std::shared_ptr<IntervalBlocks> Ptr;

  Files::CellStore::ReadersPtr      cellstores;
  Files::CommitLog::Fragments::Ptr  commitlog;


  class Block : public std::enable_shared_from_this<Block> {
    public:
    typedef std::shared_ptr<Block> Ptr;

    enum State {
      NONE,
      LOADING,
      LOADED
    };

    Files::CellsBlock::Ptr    cells_block;   
    IntervalBlocks::Ptr       blocks;
        
    Block(Files::CellsBlock::Ptr cells_block, IntervalBlocks::Ptr blocks,
          State state=State::NONE)
          : cells_block(cells_block), blocks(blocks), m_state(state) {
    }

    virtual ~Block() {}

    bool loaded() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_state = Block::State::LOADED;
    }
    
    size_t release() {
      return 0;
    }

    bool add_logged(const DB::Cells::Cell& cell) { 
      if(!cells_block->interval.consist(cell.key))
        return false;

      cells_block->cells.add(cell);
      return true;
    }

    bool scan(DB::Cells::ReqScan::Ptr req) {
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
        
        m_queue.push(new Callback(req, shared_from_this()));

        if((run = state == Block::State::NONE))
          m_state = Block::State::LOADING;
      }

      if(run) {
        bool expected = false;
        for(auto& cs : *blocks->cellstores.get()) {
          if(!cs->interval.consist(cells_block->interval))
            continue;
          expected = true;
          cs->load_cells(
            cells_block, 
            [ptr=shared_from_this()](int err) {
              ptr->loaded_cellstores(err);
            }
          );
        }
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
        [ptr=shared_from_this()](int err){
          ptr->loaded_logs(err);
        }
      );
    }

    void loaded_logs(int& err) {
      //std::cout << " IntervalBlock::loaded_logs\n";
      blocks->split(this);
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
          cb = m_queue.front();
        }

        cb->call(err);
        delete cb;
      
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_queue.pop();
          if(m_queue.empty())
            return;
        }
      }
    }

    Ptr split(size_t keep=100000) {
      //std::cout << " IntervalBlock::split(THIS)\n";
      std::lock_guard<std::mutex> lock(m_mutex);

      Ptr blk = std::make_shared<Block>(
        std::make_shared<Files::CellsBlock>(
          DB::Cells::Interval(), 
          Env::Schemas::get()->get(blocks->commitlog->range->cid)
        ), 
        blocks,
        Block::State::LOADED
      );

      cells_block->cells.write_and_free_on_key_offset(
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

    const std::string to_string() {
      std::lock_guard<std::mutex> lock(m_mutex);

      std::string s("Block(state=");
      s.append(std::to_string((uint8_t)m_state));
      s.append(" queue=");
      s.append(std::to_string(m_queue.size()));
      s.append(" ");
      s.append(cells_block->to_string());
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
          ptr->blocks->scan(req, ptr.get()); 
        }
      }
    };
    
    std::mutex             m_mutex;
    State                  m_state;
    std::queue<Callback*>  m_queue;

  }; // class Block


  // scan >> blk match >> load-cs + load+logs 

  IntervalBlocks(Files::CellStore::ReadersPtr cellstores,    
                 Files::CommitLog::Fragments::Ptr commitlog)
                : cellstores(cellstores), commitlog(commitlog) {
  }
  
  virtual ~IntervalBlocks(){
  }
 
  void stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    int err = Error::RS_NOT_READY;
    for(auto& blk : m_blocks)
      blk->run_queue(err);
  }
  
  void add_logged(const DB::Cells::Cell& cell) {

    commitlog->add(cell);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto idx = 0; idx<m_blocks.size(); idx++) {
      if(m_blocks[idx]->loaded())
        split(idx);

      if(m_blocks[idx]->add_logged(cell) && !cell.on_fraction)
        break;
    }

  }

  void scan(DB::Cells::ReqScan::Ptr req, Block* blk_ptr = nullptr) {
    //std::cout << "IntervalBlocks::scan "<<  to_string() << " " << req->to_string() << "\n";
    int err = Error::OK;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(cellstores->empty())
        err = Error::RS_NOT_LOADED_RANGE;

      else if(m_blocks.empty()) 
        init(err);
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
            if(blk_ptr == eval.get())
              flw = true;
            continue;
          }
          if((req->spec->offset_key.empty() 
              || eval->cells_block->interval.is_in_end(req->spec->offset_key))
             && eval->cells_block->interval.includes(req->spec)) {
            blk = eval;
            blk_ptr = eval.get();
            break;
          }
        }
        if(blk == nullptr)  
          break;
        //std::cout << "IntervalBlocks::scan count=" << m_blocks.size() << " "
        //            <<  blk->to_string() << "\n";
      }

      if(blk->scan(req))
        return;
      
      //std::cout << "IntervalBlocks::scan "<<  req->to_string() << "\n";
      if(req->reached_limits())
        break;
    }

    req->response(err);
  }

  size_t cells_count() {
    size_t sz = 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& blk : m_blocks) {
      sz += blk->cells_block->cells.size();
    }
    return sz;
  }
  
  void split() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto idx = 0; idx<m_blocks.size(); idx++) {
      if(m_blocks[idx]->loaded())
        split(idx);
    }
  }

  void split(Block* ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto idx = 0; idx<m_blocks.size(); idx++) {
      if(ptr == m_blocks[idx].get()) {
        split(idx);
        return;
      }
    }
  }

  size_t release(size_t bytes=0) {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t released = 0;
    
    // released += cellstores->release(bytes);
    if(bytes && released >= bytes)
      return released;
    // released += commitlog->release(bytes);
    if(bytes && released >= bytes)
      return released;

    for(auto blk : m_blocks) {
      released += blk->release();
      if(bytes && released >= bytes)
        return released;
    }
    return released;
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
    s.append("])");
    return s;
  } 

  private:
  
  void init(int& err) {
    DB::SchemaPtr schema = Env::Schemas::get()->get(commitlog->range->cid);

    for(auto& cs : *cellstores.get()) {
      if(cs->blocks.empty()) {
        cs->load_blocks_index(err, true);
        if(err) {
          m_blocks.clear();
          return;
        }
      }

      for(auto& blk : cs->blocks) {
        //std::cout << "m_blocks.push_back " << blk->to_string() << "\n";
        m_blocks.push_back(
          std::make_shared<Block>(
            std::make_shared<Files::CellsBlock>(blk->interval, schema), 
            shared_from_this()
          )
        );
      }
    }

    if(commitlog->range->is_any_begin())
      m_blocks.front()->cells_block->interval.key_begin.free();
    if(commitlog->range->is_any_end()) 
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