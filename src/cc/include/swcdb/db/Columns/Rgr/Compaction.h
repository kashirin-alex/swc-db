/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Compaction_h
#define swcdb_lib_db_Columns_Rgr_Compaction_h

#include "swcdb/db/Protocol/Mngr/req/RangeCreate.h"
#include "swcdb/db/Protocol/Mngr/req/RangeUnloaded.h"
#include "swcdb/db/Protocol/Mngr/req/RangeRemove.h"


namespace SWC { namespace server { namespace Rgr {

class Compaction final {
  public:

  typedef Compaction* Ptr;

  Compaction() 
            : m_check_timer(
                asio::high_resolution_timer(
                  *RangerEnv::maintenance_io()->ptr())),
              m_run(true), m_running(0), m_scheduled(false),
              m_idx_cid(0), m_idx_rid(0), 
              cfg_check_interval(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.compaction.check.interval")) {
  }
  
  virtual ~Compaction(){}
 
  Ptr ptr() {
    return this;
  }

  void stop() {
    {
      std::scoped_lock lock(m_mutex);
      m_run = false;
      m_check_timer.cancel();
    }
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    if(m_running) 
      m_cv.wait(lock_wait, [&running=m_running](){return !running;});  
  }

  void schedule() {
    schedule(cfg_check_interval->get());
  }
  
  void schedule(uint32_t t_ms) {
    std::scoped_lock lock(m_mutex);
    _schedule(t_ms);
  }

  const bool stopped() {
    std::scoped_lock lock(m_mutex);
    return !m_run;
  }
  
  void run(bool continuing=false) {
    {
      std::scoped_lock lock(m_mutex); 
      if(!m_run || (!continuing && m_scheduled))
        return;
      m_scheduled = true;
    }

    Column::Ptr col     = nullptr;
    Range::Ptr range  = nullptr;
    size_t ram = 0;
    for(;;) {
      
      if((col = RangerEnv::columns()->get_next(m_idx_cid)) == nullptr)
        break;
      if(col->removing()){
        ++m_idx_cid;
        continue;
      }

      if((range = col->get_next(m_idx_rid)) == nullptr) {
        ++m_idx_cid;
        continue;
      }
      ++m_idx_rid;
      if(!range->compact_possible())
        continue;

      if(!m_run)
        break;
      {
        std::scoped_lock lock(m_mutex); 
        ++m_running;
      }
      asio::post(
        *RangerEnv::maintenance_io()->ptr(), [this, range](){ compact(range); }
      );
      
      {
        std::scoped_lock lock(m_mutex); 
        if(m_running == RangerEnv::maintenance_io()->get_size())
          return;
      }
    }
    
    {
      std::scoped_lock lock(m_mutex); 
      if(m_running)
        return;
    }
    compacted();
  }

  void compact(Range::Ptr range) {

    if(!range->is_loaded() || !m_run)
      return compacted(range);

    auto& commitlog  = range->blocks.commitlog;

    uint32_t cs_size = range->cfg->cellstore_size(); 
    uint32_t blk_size = range->cfg->block_size();
    uint8_t perc = range->cfg->compact_percent(); 
    uint32_t allow_sz = (cs_size  / 100) * perc; 

    size_t value;
    bool do_compaction;
    std::string need;
    if(do_compaction = range->compact_required())
      need.append("Required");
    else if(do_compaction = (value = commitlog.size_bytes(true)) >= allow_sz) {
      need.append("LogBytes=");
      need.append(std::to_string(value-allow_sz));
    } else if(do_compaction = (value = commitlog.size()) >= cs_size/blk_size) {
      need.append("LogCount=");
      need.append(std::to_string(value-cs_size/blk_size));
    } else if(do_compaction = range->blocks.cellstores.need_compaction(
          cs_size + allow_sz,  blk_size + (blk_size / 100) * perc)) {
      need.append("CsResize");
    }

    if(!do_compaction || !m_run)
      return compacted(range);
      
    range->compacting(Range::Compact::COMPACTING);
    
    SWC_LOGF(
      LOG_INFO, 
      "COMPACT-STARTED %d/%d %s", 
      range->cfg->cid, range->rid,
      need.c_str()
    );

    range->blocks.wait_processing();
    
    auto req = std::make_shared<CompactScan>(
      ptr(),
      range, 
      cs_size, 
      range->cfg->file_replication(), 
      blk_size, 
      range->cfg->block_cells(), 
      range->cfg->block_enc(), 
      range->cfg->cell_versions(), 
      range->cfg->cell_ttl(), 
      range->cfg->column_type()
    );
    commitlog.commit_new_fragment(true);
    commitlog.get(req->fragments_old); 
    // fragments for deletion at finalize-compaction 

    range->scan_internal(req);
  }

  class CompactScan : public DB::Cells::ReqScan {
    public:
  
    typedef std::shared_ptr<CompactScan>          Ptr;
    std::vector<Files::CommitLog::Fragment::Ptr>  fragments_old;
    std::atomic<size_t> total_cells = 0;

    CompactScan(Compaction::Ptr compactor, Range::Ptr range,
                const uint32_t cs_size, const uint8_t cs_replication,
                const uint32_t blk_size, const uint32_t blk_cells, 
                const Types::Encoding blk_encoding,
                uint32_t cell_versions, uint32_t cell_ttl, 
                Types::Column col_type) 
                : compactor(compactor), range(range),
                  cs_size(cs_size), 
                  cs_replication(cs_replication), 
                  blk_size(blk_size), blk_cells(blk_cells), 
                  blk_encoding(blk_encoding),
                  cell_versions(cell_versions),
                  cell_ttl(cell_ttl),
                  col_type(col_type) {
      make_cells();
    }

    virtual ~CompactScan() { 
      if(last_cell)
        delete last_cell;
    }

    Ptr shared() {
      return std::dynamic_pointer_cast<CompactScan>(shared_from_this());
    }

    void make_cells() {
      cells.reset(
        cell_versions, 
        cell_ttl, 
        col_type
      );
      cells.reserve(blk_cells);
    }

    bool reached_limits() override {
      return m_stopped 
          || cells.size_bytes() >= blk_size 
          || cells.size() >= blk_cells;
    }

    void response(int &err) override {
      if(m_stopped || !ready(err))
        return;

      total_cells += cells.size();
      SWC_LOGF(LOG_INFO, "COMPACT-PROGRESS %d/%d cells=%lld",  
               range->cfg->cid, range->rid, total_cells.load());

      {
        std::scoped_lock lock(m_mutex);
        if(cells.empty()) {
          if(m_writing)
            m_queue.push(nullptr);
          else 
            finalize();
          return;
        }
        m_getting = false;
      }

      auto selected_cells = new DB::Cells::Vector(cells);
      make_cells();

      auto last = selected_cells->back();
      spec.offset_key.copy(last->key);
      spec.offset_rev = last->timestamp;
      
      { 
        std::scoped_lock lock(m_mutex);
        m_queue.push(selected_cells);
        if(m_writing) 
          return;
        m_writing = true;
      }
      asio::post(
        *RangerEnv::maintenance_io()->ptr(), 
        [ptr=shared()](){ ptr->process(); 
      });
      request_more();
    }

    private:

    void request_more() {
      {
        std::scoped_lock lock(m_mutex);
        if(m_getting 
          || (!m_queue.empty() 
              && (m_queue.size() >= RangerEnv::maintenance_io()->get_size()
                  || m_queue.size() > Env::Resources.avail_ram()/blk_size )))
          return;
        m_getting = true;
      }

      asio::post(
        *RangerEnv::maintenance_io()->ptr(), 
        [ptr=shared()](){
          ptr->range->scan_internal(ptr->get_req_scan());
        }
      );
    }

    void process() {
      int err = Error::OK;
      DB::Cells::Vector* selected_cells;
      for(;;) {
        {
          std::scoped_lock lock(m_mutex);
          selected_cells = m_queue.front();
        }
        if(selected_cells == nullptr) {
          finalize();
          return;
        }
        if(m_stopped)
          return;
        
        request_more();

        write_cells(err, selected_cells);
        delete selected_cells;

        if(m_stopped)
          return;
        if(err || !range->is_loaded() || compactor->stopped())
          return quit();

        {
          std::scoped_lock lock(m_mutex);
          m_queue.pop();
          m_writing = !m_queue.empty();
          if(!m_writing)
            break;
        }
      }
      request_more();
    }

    uint32_t create_cs(int& err) { 
      if(!tmp_dir) { 
        Env::FsInterface::interface()->rmdir(
          err, range->get_path(Range::cellstores_tmp_dir));
        err = Error::OK;
        Env::FsInterface::interface()->mkdirs(
          err, range->get_path(Range::cellstores_tmp_dir));
        tmp_dir = true;
        if(err)
          return 0;
      }

      uint32_t id = cellstores.size()+1;
      cs_writer = std::make_shared<Files::CellStore::Write>(
        id, 
        range->get_path_cs_on(Range::cellstores_tmp_dir, id), 
        cell_versions,
        blk_encoding
      );
      cs_writer->create(err, -1, cs_replication, blk_size);
      return id;

    }

    void write_cells(int& err, DB::Cells::Vector* selected_cells) {
      
      if(cs_writer == nullptr) {
        uint32_t id = create_cs(err);
        if(err)
          return;

        if(id == 1 && range->is_any_begin()) {
          DB::Cells::Interval blk_intval;
          DynamicBuffer buff;
          uint32_t cell_count = 0;
          selected_cells->write_and_free(buff, cell_count, blk_intval, 0, 1);
          // 1st block of begin-any set with key_end as first cell
          blk_intval.key_begin.free();

          if(range->is_any_end() && selected_cells->empty()) {
            // there was one cell
            blk_intval.key_end.free(); 
          }
          
          cs_writer->block(err, blk_intval, buff, cell_count);

          if(err || selected_cells->empty())
            return;
        }
      }

      DB::Cells::Interval blk_intval;
      DynamicBuffer buff;
      uint32_t cell_count = 0;

      if(range->is_any_end()) {
        if(last_cell) {
          ++cell_count;
          last_cell->write(buff);
          blk_intval.expand(*last_cell);
          last_cell->key.align(
            blk_intval.aligned_min, blk_intval.aligned_max);
          delete last_cell;
        }
        selected_cells->pop_back(last_cell);
        //last block of end-any to be set with first key as last cell
      }
      selected_cells->write_and_free(buff, cell_count, blk_intval, 0, 0);

      if(buff.fill()) {
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
      if(cellstores.empty())
        range->get_prev_key_end(cs_writer->prev_key_end);
      else
        cs_writer->prev_key_end.copy(cellstores.back()->interval.key_end);

      cs_writer->finalize(err);
      cellstores.push_back(cs_writer);
      cs_writer = nullptr;
    }

    void finalize() {
      
      if(!range->is_loaded())
        return quit();

      int err = Error::OK;
      bool empty_cs = false;

      if(range->is_any_end() && last_cell) {
        // last block of end-any set with key_begin with last cell
        if(cs_writer == nullptr) {       
          uint32_t id = create_cs(err);
          if(err)
            return quit();
        }
        DB::Cells::Interval blk_intval;
        blk_intval.expand(*last_cell);
        blk_intval.key_end.free();
        last_cell->key.align(blk_intval.aligned_min, blk_intval.aligned_max);

        uint32_t cell_count = 1;
        DynamicBuffer buff;
        last_cell->write(buff);
        delete last_cell;
        last_cell = nullptr;
        cs_writer->block(err, blk_intval, buff, cell_count);
     
      } else if(!cellstores.size() && cs_writer == nullptr) {
        // as an initial empty range cs with range intervals
        empty_cs = true;
        uint32_t id = create_cs(err);
        if(err)
          return quit();
        uint32_t cell_count = 0;
        DB::Cells::Interval blk_intval;
        range->get_interval(blk_intval.key_begin, blk_intval.key_end);
        DynamicBuffer buff;
        cs_writer->block(err, blk_intval, buff, cell_count);
      }
      if(err)
        return quit();

      if(cs_writer != nullptr) {
        if(!cs_writer->size)
          return quit();
        add_cs(err);
        if(err)
          return quit();
      }

      auto max = range->cfg->cellstore_max();
      if(cellstores.size() > 1 && cellstores.size() >= max) {
        auto c = cellstores.size()/2;
        if(c > 1)
          --c;
        split_option: 
          auto it = cellstores.begin()+c;
          do {
            if(!(*it)->interval.key_begin.equal((*(it-1))->interval.key_end))
              break;
          } while(++it < cellstores.end());

        if(it == cellstores.end() && c > 1) {
          c = 1; 
          goto split_option;
        }
        if(it != cellstores.end()) {
          mngr_create_range(it-cellstores.begin());
          return;
        }
        SWC_LOGF(
          LOG_WARN, 
          "COMPACT-SPLIT %d/%d fail(versions-over-cs) cs-count=%d", 
          range->cfg->cid, range->rid, cellstores.size()
        );
      }
      
      apply_new(empty_cs);

    }

    void mngr_create_range(uint32_t split_at) {
      Protocol::Mngr::Req::RangeCreate::request(
        range->cfg->cid,
        RangerEnv::rgr_data()->id,
        [split_at, cid=range->cfg->cid, ptr=shared()]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, 
         Protocol::Mngr::Params::RangeCreateRsp rsp) {
          
          SWC_LOGF(
            LOG_DEBUG, 
            "Compact::Mngr::Req::RangeCreate err=%d(%s) %d/%d", 
            rsp.err, Error::get_text(rsp.err), cid, rsp.rid
          );

          if(rsp.err && 
             rsp.err != Error::COLUMN_NOT_EXISTS &&
             rsp.err != Error::COLUMN_MARKED_REMOVED &&
             rsp.err != Error::COLUMN_NOT_READY) {
            req->request_again();
            return;
          }
          if(rsp.rid && (!rsp.err || rsp.err == Error::COLUMN_NOT_READY))
            ptr->split(rsp.rid, split_at);
          else 
            ptr->apply_new();
        }
      );
    }

    void mngr_remove_range(Range::Ptr new_range) {
      std::promise<void> res;
      Protocol::Mngr::Req::RangeRemove::request(
        new_range->cfg->cid,
        new_range->rid,
        [new_range, await=&res]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, 
         Protocol::Mngr::Params::RangeRemoveRsp rsp) {
          
          SWC_LOGF(
            LOG_DEBUG, 
            "Compact::Mngr::Req::RangeRemove err=%d(%s) %d/%d", 
            rsp.err, Error::get_text(rsp.err), 
            new_range->cfg->cid, new_range->rid
          );
          
          if(rsp.err && 
             rsp.err != Error::COLUMN_NOT_EXISTS &&
             rsp.err != Error::COLUMN_MARKED_REMOVED &&
             rsp.err != Error::COLUMN_NOT_READY) {
             req->request_again();
          } else {
            await->set_value();
          }
        }
      );
      res.get_future().get();
    }

    void split(int64_t new_rid, uint32_t split_at) {
      int err = Error::OK;
      Column::Ptr col = RangerEnv::columns()->get_column(err, range->cfg->cid);
      if(col == nullptr || col->removing())
        return quit();

      SWC_LOGF(LOG_INFO, "COMPACT-SPLIT %d/%d new-rid=%d", 
               range->cfg->cid, range->rid, new_rid);

      auto new_range = col->get_range(err, new_rid, true);
      if(!err)
        new_range->create_folders(err);
      if(err) {
        SWC_LOGF(LOG_INFO, "COMPACT-SPLIT cancelled err=%d %d/%d new-rid=%d", 
                err, range->cfg->cid, range->rid, new_rid);
        err = Error::OK;
        col->remove(err, new_rid, false);
        mngr_remove_range(new_range);
        return apply_new();
      }
      
      Files::CellStore::Writers new_cellstores;
      auto it = cellstores.begin()+split_at;
      new_cellstores.assign(it, cellstores.end());
      cellstores.erase(it, cellstores.end());
      
      SWC_ASSERT(cellstores.size());
      SWC_ASSERT(new_cellstores.size());

      new_range->create(err, new_cellstores);
      if(!err)
        range->apply_new(err, cellstores, fragments_old);

      if(err) {
        err = Error::OK;
        col->remove(err, new_rid);
        mngr_remove_range(new_range);
        return quit();
      }

      new_range = nullptr;
      col->unload(
        new_rid, 
        [new_rid, cid=range->cfg->cid](int err) { 
          Protocol::Mngr::Req::RangeUnloaded::request(
            cid, new_rid,
            [cid, new_rid](Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, 
             Protocol::Mngr::Params::RangeUnloadedRsp rsp) {
          
              SWC_LOGF(
                LOG_DEBUG, 
                "Compact::Mngr::Req::RangeUnloaded err=%d(%s) %d/%d", 
                rsp.err, Error::get_text(rsp.err), cid, new_rid
              );
              if(rsp.err && 
                 rsp.err != Error::COLUMN_NOT_EXISTS &&
                 rsp.err != Error::COLUMN_MARKED_REMOVED &&
                 rsp.err != Error::COLUMN_NOT_READY) {
                req->request_again();
              }
            }
          );
        }
      );

      range->compact_require(range->cfg->cellstore_max() > cellstores.size());
      
      SWC_LOGF(LOG_INFO, "COMPACT-FINISHED %d/%d", 
               range->cfg->cid, range->rid);
      compactor->compacted(range, true);
    }

    void apply_new(bool clear = false) {
      int err = Error::OK;
      range->apply_new(err, cellstores, fragments_old);
      if(err)
        return quit();

      range->compact_require(false);
      
      SWC_LOGF(LOG_INFO, "COMPACT-FINISHED %d/%d", 
               range->cfg->cid, range->rid);
      compactor->compacted(range, clear);
    }

    void quit() {
      m_stopped = true;
      int err = Error::OK;
      if(cs_writer != nullptr) {
        cs_writer->remove(err);
        cs_writer = nullptr;
      }
      if(tmp_dir) {
        tmp_dir = false;
        Env::FsInterface::interface()->rmdir(
          err, range->get_path(Range::cellstores_tmp_dir));
      }

      SWC_LOGF(LOG_INFO, "COMPACT-ERROR cancelled %d/%d", 
               range->cfg->cid, range->rid);
      compactor->compacted(range);
    }


    Compaction::Ptr         compactor;
    Range::Ptr              range;
    const uint32_t          cs_size;
    const uint8_t           cs_replication;
    const uint32_t          blk_size;
    const uint32_t          blk_cells;
    const Types::Encoding   blk_encoding;
    const uint32_t          cell_versions;
    const uint32_t          cell_ttl; 
    const Types::Column     col_type;
    
    bool                            tmp_dir = false;
    Files::CellStore::Write::Ptr    cs_writer = nullptr;
    Files::CellStore::Writers       cellstores;
    DB::Cells::Cell*                last_cell = nullptr;

    std::mutex                      m_mutex;
    bool                            m_writing = false;
    std::queue<DB::Cells::Vector*>  m_queue;
    std::atomic<bool>               m_stopped = false;
    bool                            m_getting = false;
  };

  private:
  
  void compacted(Range::Ptr range, bool all=false) {
    if(all) {
      range->blocks.release(0);
      if(range->blocks.size())
        SWC_LOGF(LOG_ERROR, "%s", range->to_string().c_str());
      SWC_ASSERT(!range->blocks.size());

    } else if(size_t bytes = Env::Resources.need_ram()) {
      range->blocks.release(bytes);
    }
    
    range->compacting(Range::Compact::NONE);
    compacted();
  }

  void compacted() {
    std::scoped_lock lock(m_mutex);

    if(m_running && m_running-- == RangerEnv::maintenance_io()->get_size()) {
      asio::post(*RangerEnv::maintenance_io()->ptr(), [this](){ run(true); });
      return;
    } 
    if(!m_running) {
      if(!m_run) {
        m_cv.notify_all();
        return;
      }
      m_scheduled = false;
      _schedule(cfg_check_interval->get());
    }
  }

  void _schedule(uint32_t t_ms = 300000) {
    if(!m_run || m_running || m_scheduled)
      return;

    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_check_timer.expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;

    m_check_timer.cancel();
    m_check_timer.expires_from_now(set_in);

    m_check_timer.async_wait(
      [this](const asio::error_code ec) {
        if(ec != asio::error::operation_aborted){
          run();
        }
    }); 

    SWC_LOGF(LOG_DEBUG, "Ranger compaction scheduled in ms=%d", t_ms);
  }

  std::mutex                   m_mutex;
  asio::high_resolution_timer  m_check_timer;
  bool                         m_run;
  uint32_t                     m_running;
  bool                         m_scheduled;
  std::condition_variable      m_cv;
  
  size_t                       m_idx_cid;
  size_t                       m_idx_rid;

  const gInt32tPtr            cfg_check_interval;
};





}}}
#endif