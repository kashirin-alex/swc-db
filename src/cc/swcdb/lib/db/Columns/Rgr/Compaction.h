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

    ColumnPtr col     = nullptr;
    Range::Ptr range  = nullptr;

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
      if(!range->is_loaded())
        continue;
      
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
      return compacted();
       
    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
    Files::CommitLog::Fragments::Ptr  log = range->get_log();

    uint32_t cs_size = cfg_cs_sz->get(); 
    uint32_t blk_size = schema->blk_size ? 
                        schema->blk_size : log->cfg_blk_sz->get();
    auto blk_encoding = schema->blk_encoding != Types::Encoding::DEFAULT ?
                        schema->blk_encoding : 
                        (Types::Encoding)log->cfg_blk_enc->get();
    uint32_t perc     = cfg_compact_percent->get(); 
    // % of size of either by cellstore or block

    uint32_t allowed_sz_cs  = (cs_size  / 100) * perc;
    uint32_t allowed_sz_blk = (blk_size / 100) * perc;

    uint32_t log_sz = log->size_bytes();
    bool do_compaction = log_sz >= allowed_sz_cs;

    std::string info_log;
    size_t cs_total_sz = 0;
    uint32_t blk_total_count = 0;

    if(!do_compaction) {
      info_log.append(" blk-avg=");
      size_t   sz;
      uint32_t blk_count;
      uint32_t cs_sz    = cs_size  + allowed_sz_cs;
      uint32_t blk_sz   = blk_size + allowed_sz_blk;

      Files::CellStore::Readers cellstores;
      range->get_cellstores(cellstores);
      for(auto& cs : cellstores) {
        sz = cs->size_bytes();
        if(!sz)
          continue;
        cs_total_sz += sz;
        blk_count = cs->blocks_count();
        blk_total_count += blk_count;
        if(sz >= cs_sz || sz/blk_count >= blk_sz) { //or by max_blk_sz
          do_compaction = true;
          break;
        }
      }
      info_log.append(std::to_string(
        blk_total_count? (cs_total_sz/blk_total_count)/1000000 : 0));
      info_log.append("MB");
    }
    
    HT_INFOF(
      "Compaction %s-range(%d,%d) log=%dMB%s"
      " allowed(cs=%dMB blk=%dMB)",
      do_compaction ? "started" : "skipped",
      range->cid, range->rid, 
      log_sz/1000000,  
      info_log.c_str(),
      allowed_sz_cs/1000000, 
      allowed_sz_blk/1000000
    );

    if(!do_compaction)
      return compacted();

    // log->compacting(); // set intermidiate current log + commit_current
    std::vector<Files::CommitLog::Fragment::Ptr> fragments;
    log->get(fragments);

    // range(scan) or cs-load + read-blocks) + log(fragments load+read)
    range->scan(
      std::make_shared<CompactScan>(
        shared_from_this(), range, cs_size, blk_size, schema
      )
    );
  }

  class CompactScan : public DB::Cells::ReqScan {
    public:
  
    typedef std::shared_ptr<CompactScan>  Ptr;

    CompactScan(Compaction::Ptr compactor, Range::Ptr range,
                const uint32_t blk_size, const uint32_t cs_size, 
                DB::SchemaPtr schema) 
                : compactor(compactor), range(range),
                  blk_size(blk_size), cs_size(cs_size), 
                  schema(schema) {
      cells = DB::Cells::Mutable::make(
        1000, 
        schema->cell_versions, 
        schema->cell_ttl, 
        schema->col_type
      );
      spec = DB::Specs::Interval::make_ptr();
    }

    virtual ~CompactScan() { }

    bool reached_limits() override {
      return blk_size <= cells->size_bytes();
    }

    void response(int &err) override {
      if(!range->is_loaded()) {
        // clear temp dir
        return compactor->compacted();
      }
      std::cout << "CompactScan::response cells="<< cells->size() << "\n";
      if(!ready(err))
        return;
      
      if(!cells->size()) {
        return finalize();
      }
      
      /// cell cs-writer
      
      // adjust spec lask_key with last-cell
      cells->free();

      range->scan(get_req_scan());
    }

    void finalize() {

      return compactor->compacted();
    }


    Compaction::Ptr   compactor;
    const uint32_t    cs_size;
    const uint32_t    blk_size;
    DB::SchemaPtr     schema;
    Range::Ptr        range;
    
    Files::CellStore::Readers cellstores;
  };

  private:
  
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