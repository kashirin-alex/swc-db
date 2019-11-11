/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Compaction_h
#define swcdb_lib_db_Columns_Rgr_Compaction_h

#include "Columns.h"


namespace SWC { namespace server { namespace Rgr {

class Compaction : public std::enable_shared_from_this<Compaction> {
  public:

  typedef std::shared_ptr<Compaction> Ptr;

  Compaction(uint32_t workers=Env::Config::settings()->get<int32_t>(
                                              "swc.rgr.maintenance.handlers"))
            : m_io(std::make_shared<IoContext>("Maintenance", workers)), 
              m_check_timer(asio::high_resolution_timer(*m_io->ptr())),
              m_run(true), running(0), m_scheduled(false),
              m_idx_cid(0), m_idx_rid(0), 
              cfg_check_interval(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.compaction.check.interval")), 
              cfg_cs_max(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.CellStore.count.max")), 
              cfg_cs_sz(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.CellStore.size.max")), 
              cfg_compact_percent(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.compaction.size.percent")) {
    m_io->run(m_io);
  }
  
  virtual ~Compaction(){}
 
  void stop() {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_run = false;
      m_check_timer.cancel();
      m_io->stop();
    }
    if(!running) 
      return;
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    m_cv.wait(lock_wait, [&running=running](){return running == 0;});  
  }

  void schedule() {
    schedule(cfg_check_interval->get());
  }
  
  void schedule(uint32_t t_ms) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _schedule(t_ms);
  }
  
  void run() {
    {
      std::lock_guard<std::mutex> lock(m_mutex); 
      if(!m_run)
        return;
      m_scheduled = true;
    }

    Column::Ptr col     = nullptr;
    Range::Ptr range  = nullptr;
    size_t ram = 0;
    for(;;) {
      
      if((col = Env::RgrColumns::get()->get_next(m_idx_cid)) == nullptr)
        break;
      if(col->removing()){
        m_idx_cid++;
        continue;
      }

      if((range = col->get_next(m_idx_rid)) == nullptr) {
        m_idx_cid++;
        continue;
      }
      m_idx_rid++;
      if(!range->is_loaded() 
        || range->compacting() 
        || range->blocks.processing())
        continue;
        
      range->compacting(Range::Compact::CHECKING);
      ++running;
      asio::post(
        *m_io->ptr(), 
        [range, ptr=shared_from_this()](){ 
          ptr->compact(range); 
        }
      );
      if(running == m_io->get_size())
        return;
    }
    
    if(!running)
      compacted();
  }

  void compact(Range::Ptr range) {

    if(!range->is_loaded())
      return compacted(range);

    auto schema     = Env::Schemas::get()->get(range->cid);
    auto log        = range->blocks.commitlog;
    auto cellstores = range->blocks.cellstores;

    uint32_t cs_size = cfg_cs_sz->get(); 
    uint32_t blk_size = schema->blk_size ? 
                        schema->blk_size : log->cfg_blk_sz->get();
    auto blk_encoding = schema->blk_encoding != Types::Encoding::DEFAULT ?
                        schema->blk_encoding : 
                        (Types::Encoding)log->cfg_blk_enc->get();
    uint32_t perc     = cfg_compact_percent->get(); 
    // % of size of either by cellstore or block
    
    uint32_t allow_sz = (cs_size  / 100) * perc; 
    uint32_t allowed_sz_cs  = cs_size + allow_sz;
    uint32_t log_sz = log->size_bytes();

    bool do_compaction = log_sz >= allowed_sz_cs
      || log->size() > allowed_sz_cs/blk_size 
      || (log_sz > allow_sz && log_sz > (cellstores->size_bytes()/100) * perc)
      || cellstores->need_compaction(
          allowed_sz_cs,  blk_size + (blk_size / 100) * perc);
    
    HT_INFOF(
      "Compaction %s-range %d/%d allow=%dMB"
      " log=%d=%d/%dMB cs=%d=%d/%dMB blocks=%d=%dMB",
      do_compaction ? "started" : "skipped",
      range->cid, range->rid, 
      allowed_sz_cs/1000000,

      log->size(),
      log->size_bytes(true)/1000000,
      log_sz/1000000,

      cellstores->size(),
      cellstores->size_bytes(true)/1000000,
      cellstores->size_bytes()/1000000,
      
      range->blocks.size(),
      range->blocks.size_bytes()/1000000
    );

    if(!do_compaction)
      return compacted(range);
    
    range->compacting(Range::Compact::COMPACTING);
    range->blocks.wait_processing();
    
    auto req = std::make_shared<CompactScan>(
      shared_from_this(), range, cs_size, blk_size, blk_encoding, schema
    );
    log->commit_new_fragment(true);
    log->get(req->fragments_old); // fragments for deletion at finalize-compaction 

    range->scan_internal(req);
  }

  class CompactScan : public DB::Cells::ReqScan {
    public:
  
    typedef std::shared_ptr<CompactScan>  Ptr;

    CompactScan(Compaction::Ptr compactor, Range::Ptr range,
                const uint32_t cs_size, const uint32_t blk_size, 
                const Types::Encoding blk_encoding,
                DB::Schema::Ptr schema) 
                : compactor(compactor), range(range),
                  cs_size(cs_size), blk_size(blk_size), 
                  blk_encoding(blk_encoding),
                  schema(schema) {
      cells = DB::Cells::Mutable::make(
        1000, 
        schema->cell_versions, 
        schema->cell_ttl, 
        schema->col_type
      );
      spec = DB::Specs::Interval::make_ptr();
      //drop_caches = true;
    }

    virtual ~CompactScan() { }

    Ptr shared() {
      return std::dynamic_pointer_cast<CompactScan>(shared_from_this());
    }

    bool reached_limits() override {
      return cells->size_bytes() >= blk_size;
    }

    void response(int &err) override {
      if(!range->is_loaded()) {
        // clear temp dir
        return compactor->compacted(range);
      }
      if(!ready(err))
        return;
      
      //std::cout << "CompactScan::response cells="<< cells->size() 
      //          << " sz=" << cells->size_bytes() << "\n";
      if(!cells->size()) {
        return finalize();
      }

      asio::post(
        *compactor->io()->ptr(), [ptr=shared()](){ptr->process_and_get_more();}
      );
    }

    void process_and_get_more() {
      DB::Cells::Cell last;
      cells->get(-1, last);
      //std::cout << "CompactScan::response last="<< last.to_string() << "\n";
      //spec->key_start.set(last.key, Condition::GE);
      //spec->ts_start.set(last.timestamp, Condition::GT);
      spec->offset_key.copy(last.key);
      spec->offset_rev = last.timestamp;

      int err = Error::OK;
      write_cells(err);
      if(err)
        return quit();
      
      cells->free();

      if(!range->is_loaded())
        return quit();

      range->scan_internal(get_req_scan());
    }

    void write_cells(int& err) {

      // if(cellstores.size() == cfg_cs_sz->get() ||
      //    new_ranges.back() == cfg_cs_sz->get()) { 
      // mngr.get_next_rid 
      // continue with adding cellstore to the new range (new_ranges.back())
      // }

      if(!tmp_dir) { 
        Env::FsInterface::interface()->rmdir(
          err, range->get_path(Range::cellstores_tmp_dir));
        err = Error::OK;
        Env::FsInterface::interface()->mkdirs(
          err, range->get_path(Range::cellstores_tmp_dir));
        tmp_dir = true;
        if(err)
          return;
      }

      if(cs_writer == nullptr) {
        uint32_t id = cellstores.size()+1;
        cs_writer = std::make_shared<Files::CellStore::Write>(
          id, 
          range->get_path_cs_on(Range::cellstores_tmp_dir, id), 
          blk_encoding
        );
        int32_t replication=-1;
        cs_writer->create(err, -1, replication, blk_size);
        if(err)
          return;

        if(id == 1 && range->is_any_begin()) {
          DB::Cells::Interval blk_intval;
          DynamicBuffer buff;
          uint32_t cell_count = 0;
          cells->write_and_free(buff, cell_count, blk_intval, 1);
          // 1st block of begin-any set with key_end as first cell
          blk_intval.key_begin.free();

          if(range->is_any_end() && !cells->size()) // there was one cell
            blk_intval.key_end.free(); 
          
          cs_writer->block(err, blk_intval, buff, cell_count);

          if(err || !cells->size())
            return;
        }
      }

      DB::Cells::Interval blk_intval;
      DynamicBuffer buff;
      uint32_t cell_count = 0;

      if(range->is_any_end()){
        if(!last_cell.key.empty()) {
          cell_count++;
          last_cell.write(buff);
          blk_intval.expand(last_cell);
        }
        cells->pop(-1, last_cell);
        //last block of end-any to be set with first key as last cell
      }
      cells->write_and_free(buff, cell_count, blk_intval, 0);

      if(buff.fill() > 0) {
        cs_writer->block(err, blk_intval, buff, cell_count);
        if(err)
          return;
      }
      if(cs_writer->size >= cs_size) {
        add_cs(err);
        if(err)
          return;
      }
    }

    void add_cs(int& err) {
      cs_writer->finalize(err);
      cellstores.push_back(cs_writer);
      cs_writer = nullptr;
    }

    void finalize() {
      std::cout << "Compact ::finalize 1\n";
      int err = Error::OK;

      if(cs_writer != nullptr) {

        if(range->is_any_end() && !last_cell.key.empty()) {
          DB::Cells::Interval blk_intval;
          blk_intval.expand(last_cell);
          blk_intval.key_end.free();

          uint32_t cell_count = 1;
          DynamicBuffer buff;
          last_cell.write(buff);
          last_cell.free();
          // last block of end-any set with key_begin as last cell
          cs_writer->block(err, blk_intval, buff, cell_count);
          if(err)
            return quit();
        }
        if(cs_writer->size == 0)
          return quit();

        add_cs(err);
        if(err)
          return quit();
      }
      
      //std::cout << "CellStore::Writers: \n";
      //for(auto cs : cellstores)
      //  std::cout << " " << cs->to_string() << "\n";

      if(cellstores.size() > 0) 
        range->apply_new(err, cellstores, fragments_old);
      if(err)
        return quit();
        
      compactor->compacted(range);

      std::cout << "Compact ::finalize 2\n";
    }

    void quit() {
      int err = Error::OK;
      if(cs_writer != nullptr) {
        cs_writer->remove(err);
        cs_writer = nullptr;
      }
      if(tmp_dir)
        Env::FsInterface::interface()->rmdir(
          err, range->get_path(Range::cellstores_tmp_dir));

      compactor->compacted(range);
    }


    Compaction::Ptr         compactor;
    Range::Ptr              range;
    const uint32_t          cs_size;
    const uint32_t          blk_size;
    const Types::Encoding   blk_encoding;
    DB::Schema::Ptr         schema;
    
    std::vector<Files::CommitLog::Fragment::Ptr> fragments_old;
    
    bool                            tmp_dir = false;
    Files::CellStore::Write::Ptr    cs_writer = nullptr;
    Files::CellStore::Writers       cellstores;
    DB::Cells::Cell                 last_cell;

    std::vector<Range::Ptr>         new_ranges;

  };

  IoContext::Ptr io() {
    return m_io;
  }

  private:
  
  void compacted(Range::Ptr range) {
    range->compacting(Range::Compact::NONE);
    size_t bytes = Env::Resources.need_ram();
    if(bytes)
      range->blocks.release(bytes);
    compacted();
  }

  void compacted() {
    //std::cout << "Compaction::compacted running=" << running.load() << "\n";

    if(running && running-- == m_io->get_size()) {
      run();
      
    } else if(!running) {
      std::lock_guard<std::mutex> lock(m_mutex); 
      m_scheduled = false;
      _schedule(cfg_check_interval->get());
    }

    if(!m_run) {
      m_cv.notify_all();
      return;
    }
  }

  void _schedule(uint32_t t_ms = 300000) {
    if(!m_run || running || m_scheduled)
      return;

    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_check_timer.expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;

    m_check_timer.cancel();
    m_check_timer.expires_from_now(set_in);

    m_check_timer.async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->run();
        }
    }); 

    HT_DEBUGF("Ranger compaction scheduled in ms=%d", t_ms);
  }

  std::mutex                   m_mutex;
  IoContext::Ptr               m_io;
  asio::high_resolution_timer  m_check_timer;
  bool                         m_run;
  std::atomic<uint32_t>        running;
  bool                         m_scheduled;
  std::condition_variable      m_cv;
  
  size_t                       m_idx_cid;
  size_t                       m_idx_rid;

  const gInt32tPtr            cfg_check_interval;
  const gInt32tPtr            cfg_cs_max;
  const gInt32tPtr            cfg_cs_sz;
  const gInt32tPtr            cfg_compact_percent;
};





}}}
#endif