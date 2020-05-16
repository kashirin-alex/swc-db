/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/req/RangeCreate.h"
#include "swcdb/db/Protocol/Mngr/req/RangeUnloaded.h"
#include "swcdb/db/Protocol/Mngr/req/RangeRemove.h"

#include "swcdb/ranger/db/CommitLogCompact.h"
#include "swcdb/ranger/db/CommitLogSplitter.h"

namespace SWC { namespace Ranger {
  
struct CompactRange::InBlock {

  InBlock(const Types::KeySeq key_seq, size_t size, InBlock* inblock = nullptr)
          : has_last(inblock != nullptr), 
            cells(size), count(0), interval(key_seq), err(Error::OK),
            last_cell(0), before_last_cell(0) {
    if(has_last)
      inblock->move_last(this);
  }

  size_t cell_avg_size() const {
    return cells.fill()/count;
  }

  void add(const DB::Cells::Cell& cell) {
    ++count;
    cells.set_mark(); // start of last cell
    cell.write(cells);

    before_last_cell = last_cell;
    last_cell = cells.mark;
  }

  void set_offset(DB::Specs::Interval& spec) const {
    DB::Cells::Cell cell;
    const uint8_t* ptr = last_cell;
    size_t remain = cells.ptr - ptr;
    cell.read(&ptr, &remain); 
    spec.offset_key.copy(cell.key);
    spec.offset_rev = cell.timestamp;
  }

  void move_last(InBlock* to) {
    const uint8_t* ptr = last_cell;
    size_t remain = cells.ptr - ptr;
    
    DB::Cells::Cell cell;
    cell.read(&ptr, &remain); 
    to->add(cell);

    --count;
    cells.ptr = last_cell;
    last_cell = before_last_cell;
    cells.mark = before_last_cell = 0;
  }

  void finalize_interval(bool any_begin, bool any_end) {    
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    bool set_begin = !any_begin;

    DB::Cells::Cell cell;
    while(remain) {
      cell.read(&ptr, &remain); 
      interval.align(cell.key);
      interval.expand(cell.timestamp);
      
      if(set_begin) {
        interval.expand_begin(cell);
        set_begin = false;
      }
    }
    if(!any_end)
      interval.expand_end(cell);
  }

  void finalize_encode(Types::Encoding encoding) {
    DynamicBuffer output;
    CellStore::Block::Write::encode(err, encoding, cells, output, count);
    if(err)
      return;
    cells.free();
    cells.take_ownership(output);
  }

  const uint8_t        has_last;
  DynamicBuffer        cells;
  uint32_t             count;
  DB::Cells::Interval  interval;

  int                  err;

  private:

  uint8_t*              last_cell;
  uint8_t*              before_last_cell;
};


CompactRange::CompactRange(Compaction::Ptr compactor, RangePtr range,
                           const uint32_t cs_size, const uint32_t blk_size) 
            : ReqScan(ReqScan::Type::COMPACTION, true, 
                      compactor->cfg_read_ahead->get()/2), 
              compactor(compactor), range(range),
              cs_size(cs_size), blk_size(blk_size), 
              blk_cells(range->cfg->block_cells()), 
              blk_encoding(range->cfg->block_enc()),
              m_inblock(new InBlock(range->cfg->key_seq, blk_size)),
              m_getting(true),
              state_default(range->blocks.cellstores.blocks_count() > 1 
                ? Range::COMPACT_COMPACTING : Range::COMPACT_PREPARING),
              req_last_time(0),
              m_chk_timer(
                asio::high_resolution_timer(*Env::IoCtx::io()->ptr())) {
  spec.flags.max_versions = range->cfg->cell_versions();
}

CompactRange::~CompactRange() { 
  if(m_inblock)
    delete m_inblock;
    
  if(!m_q_intval.empty()) do {
    if(m_q_intval.front()) delete m_q_intval.front();
  } while(m_q_intval.pop_and_more());

  if(!m_q_encode.empty()) do {
    if(m_q_encode.front()) delete m_q_encode.front();
  } while(m_q_encode.pop_and_more());

  if(!m_q_write.empty()) do {
    if(m_q_write.front()) delete m_q_write.front();
  } while(m_q_write.pop_and_more());
}

SWC_SHOULD_INLINE
CompactRange::Ptr CompactRange::shared() {
  return std::dynamic_pointer_cast<CompactRange>(shared_from_this());
}

void CompactRange::initialize() {
  range->compacting(Range::COMPACT_APPLYING);
  range->blocks.wait_processing(); // sync processing state

  if(range->blocks.commitlog.cells_count(true))
    range->blocks.commitlog.commit_new_fragment(true);

  initial_commitlog(1);
}

void CompactRange::initial_commitlog(int tnum) {
  auto ptr = shared();
  if(range->blocks.commitlog.size() < CommitLog::Fragments::MIN_COMPACT)
    return initial_commitlog_done(ptr, nullptr);

  std::vector<CommitLog::Fragments::Vec> groups;
  size_t need = range->blocks.commitlog.need_compact(
    groups, {}, CommitLog::Fragments::MIN_COMPACT);
  if(need) {
    new CommitLog::Compact(
      &range->blocks.commitlog, tnum, groups,
      [ptr] (const CommitLog::Compact* compact) {
        ptr->initial_commitlog_done(ptr, compact); 
      }
    );
  } else {
    initial_commitlog_done(ptr, nullptr);
  } 
}

void CompactRange::initial_commitlog_done(CompactRange::Ptr ptr, 
                                          const CommitLog::Compact* compact) {
  if(compact) {
    int tnum = 0;
    if(compact->nfrags > 100 ||
       compact->ngroups > CommitLog::Fragments::MIN_COMPACT ||
       compact->nfrags / compact->ngroups > CommitLog::Fragments::MIN_COMPACT)
      tnum += compact->repetition + 1;
    delete compact;
    if(tnum) {
      range->compacting(Range::COMPACT_PREPARING); // range scan can continue
      return initial_commitlog(tnum);
    }
  }

  range->blocks.commitlog.get(fragments_old); // fragments for removal

  range->compacting(state_default); // range scan &/ add can continue
  req_ts = Time::now_ns();
  progress_check_timer();

  range->scan_internal(ptr);
}

bool CompactRange::with_block() {
  return true;
}

bool CompactRange::selector(const Types::KeySeq key_seq, 
                            const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(
    key_seq, cell.key, cell.timestamp, cell.control & DB::Cells::TS_DESC);
}

bool CompactRange::reached_limits() {
  return m_stopped 
      || (m_inblock->count > 1 && 
          m_inblock->cells.fill() + m_inblock->cell_avg_size() >= blk_size)
      || m_inblock->count >= blk_cells + 1;
}

bool CompactRange::add_cell_and_more(const DB::Cells::Cell& cell) {
  auto sz = m_inblock->cells.fill();
  m_inblock->add(cell);
  profile.add_cell(m_inblock->cells.fill() - sz);
  return !reached_limits();
}

bool CompactRange::add_cell_set_last_and_more(const DB::Cells::Cell& cell) {
  auto sz = m_inblock->cells.fill();
  m_inblock->add(cell);
  profile.add_cell(m_inblock->cells.fill() - sz);
  return !reached_limits();
}

bool CompactRange::matching_last(const DB::Cell::Key& key) {
  if(!m_inblock->count)
    return false;
  DB::Cell::Key key_last;
  const uint8_t* ptr = m_inblock->cells.mark + 1; // flag offset
  size_t remain = m_inblock->cells.ptr - ptr;
  key_last.decode(&ptr, &remain, false);
  return key_last.equal(key);
}

void CompactRange::response(int &err) {
  if(m_stopped)
    return;

  total_cells = profile.cells_count;
  ++total_blocks;
  req_last_time = Time::now_ns() - req_ts;
  req_ts = Time::now_ns();

  size_t c = total_cells ? total_cells.load() : 1;
  profile.finished();
  SWC_LOGF(LOG_INFO, 
    "COMPACT-PROGRESS %d/%d blocks=%lld avg(i=%lld e=%lld w=%lld)us %s err=%d",
    range->cfg->cid, range->rid,
    total_blocks.load(), 
    (time_intval / total_blocks)/1000, 
    (time_encode / total_blocks)/1000, 
    (time_write / total_blocks)/1000, 
    profile.to_string().c_str(), err
  );

  bool finishing;
  if(finishing = !reached_limits()) {
    stop_check_timer();
    range->compacting(state_default = Range::COMPACT_APPLYING);
    range->blocks.wait_processing();
    range->blocks.commitlog.commit_new_fragment(true);
  }

  auto in_block = m_inblock->count <= 1 ? nullptr : m_inblock;
  if(in_block) {
    m_inblock = new InBlock(range->cfg->key_seq, blk_size, in_block);
    m_inblock->set_offset(spec);
  }

  if(m_q_intval.push_and_is_1st(in_block))
    asio::post(*RangerEnv::maintenance_io()->ptr(), 
      [ptr=shared()](){ ptr->process_interval(); });

  if(m_stopped || !in_block)
    return;

  finishing
    ? commitlog_done(nullptr)
    : commitlog(1);
}

bool CompactRange::is_slow_req(uint64_t& median) const {
  median = (total_cells
    ? (Time::now_ns() - profile.ts_start) / total_cells : 10000) 
    * blk_cells * 3;
  return req_last_time > median || Time::now_ns() - req_ts > median;
}

void CompactRange::commitlog(int tnum) {
  if(range->blocks.commitlog.size() - fragments_old.size()
      < CommitLog::Fragments::MIN_COMPACT)
    return commitlog_done(nullptr);

  std::vector<CommitLog::Fragments::Vec> groups;
  size_t need = range->blocks.commitlog.need_compact(
    groups, fragments_old, CommitLog::Fragments::MIN_COMPACT);
  if(need) {
    new CommitLog::Compact(
      &range->blocks.commitlog, tnum, groups,
      [ptr=shared()] (const CommitLog::Compact* compact) {
        ptr->commitlog_done(compact); 
      }
    );
  } else {
    commitlog_done(nullptr);
  } 
}

void CompactRange::commitlog_done(const CommitLog::Compact* compact) {
  if(m_stopped) {
    if(compact)
      delete compact;
    return;
  }
  if(compact) {
    int tnum = 0;
    if(compact->nfrags > 100 ||
       compact->ngroups > range->cfg->log_rollout_ratio() ||
       compact->nfrags / compact->ngroups > range->cfg->log_rollout_ratio())
      tnum += compact->repetition + 1;
    delete compact;

    uint64_t median;
    range->compacting(tnum && is_slow_req(median)
      ? Range::COMPACT_PREPARING : state_default.load() );
    if(tnum && !m_stopped && !m_chk_final && range->is_loaded())
      return commitlog(tnum);
  }
  {
    Mutex::scope lock(m_mutex);
    m_getting = false;
  }
  request_more();
}

void CompactRange::progress_check_timer() {
  if(m_stopped || m_chk_final)
    return;

  uint64_t median;
  bool slow = is_slow_req(median);
  if(!range->compacting_is(Range::COMPACT_APPLYING)) {
    range->compacting(slow? Range::COMPACT_PREPARING : state_default.load());
    request_more();
  }

  if((median /= 1000000) < 1000)
    median = 1000;
  Mutex::scope lock(m_mutex);
  m_chk_timer.expires_from_now(std::chrono::milliseconds(median));
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
  if(m_stopped)
    return;
  {
    Mutex::scope lock(m_mutex);
    if(m_getting)
      return;
    m_getting = true;
  }
  bool ram = false;
  size_t sz = m_q_write.size() + m_q_intval.size() + m_q_encode.size();
  if(sz && (sz >= compactor->cfg_read_ahead->get() || 
            (ram = sz > Env::Resources.avail_ram()/blk_size) )) {
    if(!ram || (range->blocks.release(sz * blk_size) < sz * blk_size &&
                m_q_write.size() + m_q_intval.size() + m_q_encode.size()) ) {
      Mutex::scope lock(m_mutex);
      m_getting = false;
      return;
    }
  }

  asio::post(*RangerEnv::maintenance_io()->ptr(), 
    [ptr=shared()](){ ptr->range->scan_internal(ptr->get_req_scan()); });
}

void CompactRange::process_interval() {
  auto start = Time::now_ns();
  InBlock* in_block;
  do {
    if(m_stopped) 
      return;
    if((in_block = m_q_intval.front())) {
      request_more();
      in_block->finalize_interval(false, false);
    }
    if(m_q_encode.push_and_is_1st(in_block))
      asio::post(*RangerEnv::maintenance_io()->ptr(), 
        [ptr=shared()](){ ptr->process_encode(); });
  } while(m_q_intval.pop_and_more() && !m_stopped);
  time_intval += Time::now_ns() - start;
  request_more();
}

void CompactRange::process_encode() {
  auto start = Time::now_ns();
  InBlock* in_block;
  do {
    if(m_stopped) 
      return;
    if((in_block = m_q_encode.front())) {
      request_more();
      in_block->finalize_encode(blk_encoding);
    }
    if(m_q_write.push_and_is_1st(in_block))
      asio::post(*RangerEnv::maintenance_io()->ptr(), 
        [ptr=shared()](){ ptr->process_write(); });
  } while(m_q_encode.pop_and_more() && !m_stopped);
  time_encode += Time::now_ns() - start;
  request_more();
}

void CompactRange::process_write() {
  auto start = Time::now_ns();
  int err = Error::OK;
  InBlock* in_block;
  do {
    if(m_stopped)
      return;

    if((in_block = m_q_write.front()) == nullptr) {
      finalize();
      time_write += Time::now_ns() - start;
      return;
    }

    if(in_block->err)
      return quit();

    request_more();

    write_cells(err, in_block);

    if(m_stopped)
      return;
    if(err || !range->is_loaded() || compactor->stopped())
      return quit();

    delete in_block;
  } while(m_q_write.pop_and_more() && !m_stopped);

  time_write += Time::now_ns() - start;
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
    range, 
    spec.flags.max_versions
  );
  cs_writer->create(err, -1, range->cfg->file_replication(), blk_size);

  uint32_t portion = range->cfg->compact_percent()/10;
  if(!portion) portion = 1;
  if(id == range->cfg->cellstore_max() * portion) {
    stop_check_timer();
    // mitigate add req. total workload
    range->compacting(state_default = Range::COMPACT_PREPARING);
  }
  return id;

}

void CompactRange::write_cells(int& err, InBlock* in_block) {
  if(cs_writer == nullptr) {
    if(create_cs(err) == 1 && range->is_any_begin())
      in_block->interval.key_begin.free();
    if(err)
      return;
  }

  cs_writer->block(err, in_block->interval, in_block->cells);
  if(err)
    return;

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

  if(m_inblock->count) {
    // first or/and last block of any-type set with empty-key
    bool any_begin = false;
    if(cs_writer == nullptr) {
      any_begin = create_cs(err) == 1 && range->is_any_begin();
      if(err)
        return quit();
    }
    m_inblock->finalize_interval(any_begin, range->is_any_end());
    cs_writer->block(
      err, m_inblock->interval, m_inblock->cells, m_inblock->count);
 
  } else if(!cellstores.size() && cs_writer == nullptr) {
    // as an initial empty range cs with range intervals
    empty_cs = true;
    uint32_t id = create_cs(err);
    if(err)
      return quit();
    range->get_interval(
      m_inblock->interval.key_begin, m_inblock->interval.key_end);
    cs_writer->block(
      err, m_inblock->interval, m_inblock->cells, m_inblock->count);
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

  bool ok;
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

    if((ok = it != cellstores.end()) && 
       (*it)->size >= (cs_size/100) * range->cfg->compact_percent()) {
      mngr_create_range(it-cellstores.begin());
      return;
    }
    if(!ok)
      SWC_LOGF(LOG_WARN, 
        "COMPACT-SPLIT %d/%d fail(versions-over-cs) cs-count=%d",
        range->cfg->cid, range->rid, cellstores.size());
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
      
      SWC_LOGF(LOG_DEBUG, 
        "Compact::Mngr::Req::RangeCreate err=%d(%s) %d/%d", 
        rsp.err, Error::get_text(rsp.err), cid, rsp.rid);

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
      
      SWC_LOGF(LOG_DEBUG, 
        "Compact::Mngr::Req::RangeRemove err=%d(%s) %d/%d", 
        rsp.err, Error::get_text(rsp.err), 
        new_range->cfg->cid, new_range->rid);
      
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

  int64_t ts = Time::now_ns();
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
      
          SWC_LOGF(LOG_DEBUG, 
            "Compact::Mngr::Req::RangeUnloaded err=%d(%s) %d/%d", 
            rsp.err, Error::get_text(rsp.err), cid, new_rid);
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

  SWC_LOGF(LOG_INFO, "COMPACT-SPLITTED %d/%d took=%lldns new-end=%s", 
            range->cfg->cid, range->rid, Time::now_ns() - ts,
            cellstores.back()->interval.key_end.to_string().c_str());
  finished(true);
}

void CompactRange::apply_new(bool clear) {
  int err = Error::OK;
  range->apply_new(err, cellstores, fragments_old, true);
  if(err)
    return quit();

  finished(clear);
}

void CompactRange::finished(bool clear) {
  range->compact_require(false);
  
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FINISHED %d/%d cells=%lld blocks=%lld "
    "(total=%lld intval=%lld encode=%lld write=%lld)ms %s",
    range->cfg->cid, range->rid, total_cells.load(), total_blocks.load(),
    (Time::now_ns() - profile.ts_start)/1000000, 
    time_intval/1000000, time_encode/1000000, time_write/1000000,
    profile.to_string().c_str()
  );

  compactor->compacted(range, clear);
}

void CompactRange::quit() {
  m_stopped = true;

  stop_check_timer();
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
  auto ptr = shared();
  for(int chk = 0; ptr.use_count() > 2; ++chk) {
    if(chk == 3000) {
      SWC_LOGF(LOG_INFO, "COMPACT-STOPPING %d/%d use_count=%d", 
                range->cfg->cid, range->rid, ptr.use_count());
      chk = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  SWC_LOGF(LOG_INFO, "COMPACT-ERROR cancelled %d/%d", 
           range->cfg->cid, range->rid);
  compactor->compacted(range);
}






}}