/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/req/RangeCreate.h"
#include "swcdb/db/Protocol/Mngr/req/RangeUnloaded.h"
#include "swcdb/db/Protocol/Mngr/req/RangeRemove.h"

#include "swcdb/ranger/db/CommitLogSplitter.h"

namespace SWC { namespace Ranger {

CompactRange::CompactRange(
            Compaction::Ptr compactor, RangePtr range,
            const uint32_t cs_size, const uint8_t cs_replication,
            const uint32_t blk_size, const uint32_t blk_cells, 
            const Types::Encoding blk_encoding,
            uint32_t cell_versions, uint64_t cell_ttl, 
            Types::Column col_type) 
            : compactor(compactor), range(range),
              cs_size(cs_size), 
              cs_replication(cs_replication), 
              blk_size(blk_size), blk_cells(blk_cells), 
              blk_encoding(blk_encoding),
              cell_versions(cell_versions),
              cell_ttl(cell_ttl),
              col_type(col_type),
              m_ts_start(Time::now_ns()),
              m_chk_timer(
                asio::high_resolution_timer(*Env::IoCtx::io()->ptr())) {
}

CompactRange::~CompactRange() { 
  if(last_cell)
    delete last_cell;
}

CompactRange::Ptr CompactRange::shared() {
  return std::dynamic_pointer_cast<CompactRange>(shared_from_this());
}

void CompactRange::initialize() {
  cells.configure(
    cell_versions, 
    cell_ttl, 
    col_type
  );
  cells.reserve(blk_cells);

  m_req_ts = Time::now_ns();
  progress_check_timer();
}

bool CompactRange::reached_limits() {
  return m_stopped 
      || cells.size_bytes() >= blk_size 
      || cells.size() >= blk_cells;
}

bool CompactRange::selector(const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(
    cell.key, cell.timestamp, cell.control & DB::Cells::TS_DESC);
}

void CompactRange::response(int &err) {
  if(m_stopped)
    return;

  total_cells += cells.size();
  SWC_LOGF(LOG_INFO, "COMPACT-PROGRESS %d/%d cells=%lld avg=%lldns",  
           range->cfg->cid, range->rid, 
           total_cells.load(), 
           (Time::now_ns() - m_ts_start) 
            / (total_cells ? total_cells.load() : (size_t)1));

  if(!reached_limits()) {
    stop_check_timer();
    range->compacting(Range::COMPACT_APPLYING);
  }

  auto selected_cells = cells.empty() 
                        ? nullptr : new DB::Cells::Result(cells);
  bool finished;
  {
    Mutex::scope lock(m_mutex);
    if(selected_cells) {
      auto& last = selected_cells->back();
      spec.offset_key.copy(last->key);
      spec.offset_rev = last->timestamp;
      m_getting = false;
    }
    if(!(finished = !m_writing && !selected_cells)) {
      m_queue.push(selected_cells);
      if(m_writing)
        return;
      m_writing = true;
    }
  }
  
  if(finished) {
    finalize();
    return;
  }

  asio::post(
    *RangerEnv::maintenance_io()->ptr(), 
    [ptr=shared()](){ ptr->process(); 
  });

  request_more();
}

void CompactRange::progress_check_timer() {
  uint64_t req_ts;
  {
    Mutex::scope lock(m_mutex);
    req_ts = m_req_ts;
  }
  if(m_stopped || m_chk_final)
    return;

  uint64_t median = (total_cells.load() 
    ? (Time::now_ns() - m_ts_start) / total_cells.load() : 10000) * blk_cells;

  range->compacting((Time::now_ns() - req_ts > median * 10) 
    ? Range::COMPACT_PREPARING  // mitigate add req. workload
    : Range::COMPACT_COMPACTING // range scan & add reqs can continue
  );

  if((median /= 1000000) < 1000)
    median = 1000;
  Mutex::scope lock(m_mutex);
  m_chk_timer.expires_from_now(std::chrono::milliseconds(median*2));
  m_chk_timer.async_wait(
    [ptr=shared()](const asio::error_code ec) {
      if(ec == asio::error::operation_aborted)
        return;
      ptr->progress_check_timer();
    }
  );
}

void CompactRange::stop_check_timer() {
  if(!m_chk_final) {
    m_chk_final = true;
    Mutex::scope lock(m_mutex);
    m_chk_timer.cancel();
  }
}

void CompactRange::request_more() {
  {
    Mutex::scope lock(m_mutex);
    if(m_getting 
      || (!m_queue.empty() 
          && (m_queue.size() >= RangerEnv::maintenance_io()->get_size()
              || m_queue.size() > Env::Resources.avail_ram()/blk_size )))
      return;
    m_getting = true;
    m_req_ts = Time::now_ns();
  }

  asio::post(
    *RangerEnv::maintenance_io()->ptr(), 
    [ptr=shared()](){
      ptr->range->scan_internal(ptr->get_req_scan());
    }
  );
}

void CompactRange::process() {
  int err = Error::OK;
  DB::Cells::Result* selected_cells;
  for(;;) {
    {
      Mutex::scope lock(m_mutex);
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
      Mutex::scope lock(m_mutex);
      m_queue.pop();
      m_writing = !m_queue.empty();
      if(!m_writing)
        break;
    }
  }
  request_more();
}

uint32_t CompactRange::create_cs(int& err) { 
  if(!tmp_dir) { 
    Env::FsInterface::interface()->rmdir(
      err, range->get_path(Range::CELLSTORES_TMP_DIR));
    err = Error::OK;
    Env::FsInterface::interface()->mkdirs(
      err, range->get_path(Range::CELLSTORES_TMP_DIR));
    tmp_dir = true;
    if(err)
      return 0;
  }

  uint32_t id = cellstores.size()+1;
  cs_writer = std::make_shared<CellStore::Write>(
    id, 
    range->get_path_cs_on(Range::CELLSTORES_TMP_DIR, id), 
    cell_versions,
    blk_encoding
  );
  cs_writer->create(err, -1, cs_replication, blk_size);

  uint32_t portion = range->cfg->compact_percent()/10;
  if(!portion) portion = 1;
  if(id == range->cfg->cellstore_max() * portion) {
    stop_check_timer();
    range->compacting(Range::COMPACT_PREPARING);
  }
  return id;

}

void CompactRange::write_cells(int& err, DB::Cells::Result* selected_cells) {

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
    last_cell = selected_cells->takeout_end(1);
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

void CompactRange::add_cs(int& err) {
  if(cellstores.empty())
    range->get_prev_key_end(cs_writer->prev_key_end);
  else
    cs_writer->prev_key_end.copy(cellstores.back()->interval.key_end);

  cs_writer->finalize(err);
  cellstores.push_back(cs_writer);
  cs_writer = nullptr;
}

void CompactRange::finalize() {
  
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

  range->compacting(Range::COMPACT_APPLYING);
  range->blocks.wait_processing();

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

void CompactRange::mngr_create_range(uint32_t split_at) {
  Protocol::Mngr::Req::RangeCreate::request(
    range->cfg->cid,
    RangerEnv::rgr_data()->id,
    [split_at, cid=range->cfg->cid, ptr=shared()]
    (client::ConnQueue::ReqBase::Ptr req, 
     const Protocol::Mngr::Params::RangeCreateRsp& rsp) {
      
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

void CompactRange::mngr_remove_range(RangePtr new_range) {
  std::promise<void> res;
  Protocol::Mngr::Req::RangeRemove::request(
    new_range->cfg->cid,
    new_range->rid,
    [new_range, await=&res]
    (client::ConnQueue::ReqBase::Ptr req, 
     const Protocol::Mngr::Params::RangeRemoveRsp& rsp) {
      
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

void CompactRange::split(int64_t new_rid, uint32_t split_at) {
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
  
  CellStore::Writers new_cellstores;
  auto it = cellstores.begin()+split_at;
  new_cellstores.assign(it, cellstores.end());
  cellstores.erase(it, cellstores.end());

  new_range->create(err, new_cellstores);
  if(!err) 
    range->apply_new(err, cellstores, fragments_old, false);

  if(err) {
    err = Error::OK;
    col->remove(err, new_rid);
    mngr_remove_range(new_range);
    return quit();
  }

  SWC_LOGF(LOG_INFO, "COMPACT-SPLITTED %d/%d new-end=%s", 
            range->cfg->cid, range->rid, 
            cellstores.back()->interval.key_end.to_string().c_str());

  if(range->blocks.commitlog.cells_count()) {
    /* split latest fragments to new_range from new interval key_end */

    fragments_old.clear();
    range->blocks.commitlog.commit_new_fragment(true);
    range->blocks.commitlog.get(fragments_old); // fragments for removal
      
    CommitLog::Splitter splitter(
      cellstores.back()->interval.key_end,
      fragments_old,
      &range->blocks.commitlog,
      &new_range->blocks.commitlog
    );

    splitter.run();
    range->blocks.commitlog.remove(err, fragments_old);

    range->blocks.commitlog.commit_new_fragment(true);
    new_range->blocks.commitlog.commit_new_fragment(true);
  }
  range->expand_and_align(err, true);
  new_range->expand_and_align(err, false);
  //err = Error::OK;

  new_range = nullptr;
  col->unload(
    new_rid, 
    [new_rid, cid=range->cfg->cid](int err) { 
      Protocol::Mngr::Req::RangeUnloaded::request(
        cid, new_rid,
        [cid, new_rid](client::ConnQueue::ReqBase::Ptr req, 
         const Protocol::Mngr::Params::RangeUnloadedRsp& rsp) {
      
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
  
  SWC_LOGF(LOG_INFO, "COMPACT-FINISHED %d/%d cells=%lld took=%lldns", 
           range->cfg->cid, range->rid, 
           total_cells.load(), Time::now_ns() - m_ts_start);
  compactor->compacted(range, true);
}

void CompactRange::apply_new(bool clear) {
  int err = Error::OK;
  range->apply_new(err, cellstores, fragments_old, true);
  if(err)
    return quit();

  range->compact_require(false);
  
  SWC_LOGF(LOG_INFO, "COMPACT-FINISHED %d/%d cells=%lld took=%lldns", 
           range->cfg->cid, range->rid, 
           total_cells.load(), Time::now_ns() - m_ts_start);
  compactor->compacted(range, clear);
}

void CompactRange::quit() {
  stop_check_timer();

  m_stopped = true;
  int err = Error::OK;
  if(cs_writer != nullptr) {
    cs_writer->remove(err);
    cs_writer = nullptr;
  }
  if(tmp_dir) {
    tmp_dir = false;
    Env::FsInterface::interface()->rmdir(
      err, range->get_path(Range::CELLSTORES_TMP_DIR));
  }

  SWC_LOGF(LOG_INFO, "COMPACT-ERROR cancelled %d/%d", 
           range->cfg->cid, range->rid);
  compactor->compacted(range);
}






}}