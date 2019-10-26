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
          ptr->compacted();
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
      return;
    
    //Files::CellStore::ReadersPtr      m_cellstores;

    Files::CommitLog::Fragments::Ptr  log = range->get_log();
    size_t log_sz = log->size_bytes();
    
    std::vector<Files::CommitLog::Fragment::Ptr> fragments;
    log->get(fragments);
              
    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);

    uint32_t blk_size = schema->blk_size ? 
                        schema->blk_size : log->cfg_blk_sz->get();
    auto blk_encoding = schema->blk_encoding != Types::Encoding::DEFAULT ?
                        schema->blk_encoding : 
                        (Types::Encoding)log->cfg_blk_enc->get();


    std::cout << "Compaction cid=" << range->cid << " rid=" << range->rid 
              << " log_sz=" << log_sz 
              << " cells_count=" << log->cells_count() 
              << ", config:" 
              << " check_interval=" << cfg_check_interval->get()
              << " cs_max=" << cfg_cs_max->get()
              << " cs_sz=" << cfg_cs_sz->get()
              << " blk_sz=" << blk_size
              << " blk_enc=" << Types::to_string(blk_encoding)
              << " compact_percent=" << cfg_compact_percent->get()
              << "\n";
  

    //range->scan(all, no deletes)

  }

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