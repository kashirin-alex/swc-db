/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/RangeCreate.h"
#include "swcdb/db/Protocol/Mngr/req/RangeUnloaded.h"
#include "swcdb/db/Protocol/Mngr/req/RangeRemove.h"

#include "swcdb/ranger/db/CommitLogCompact.h"
#include "swcdb/ranger/db/CommitLogSplitter.h"
#include "swcdb/ranger/db/RangeSplit.h"

namespace SWC { namespace Ranger {

struct CompactRange::InBlock final : Core::QueuePointer<InBlock*>::Pointer {

  const bool is_last;

  SWC_CAN_INLINE
  InBlock(const DB::Types::KeySeq key_seq) noexcept
          : is_last(true),
            header(key_seq), err(Error::OK), last_cell(nullptr) {
  }

  SWC_CAN_INLINE
  InBlock(const DB::Types::KeySeq key_seq, size_t size,
          InBlock* inblock = nullptr)
          : is_last(false),
            cells(size + 1000000), header(key_seq), err(Error::OK),
            last_cell(nullptr) {
    if(inblock)
      inblock->move_last(this);
  }

  InBlock(const InBlock&) = delete;

  InBlock(const InBlock&&) = delete;

  InBlock& operator=(const InBlock&) = delete;

  //~InBlock() { }

  SWC_CAN_INLINE
  size_t cell_avg_size() const {
    return cells.fill()/header.cells_count;
  }

  SWC_CAN_INLINE
  void add(const DB::Cells::Cell& cell) {
    ++header.cells_count;
    cells.set_mark(); // start of last cell
    cell.write(cells);
    last_cell = cells.mark;
  }

  SWC_CAN_INLINE
  void set_offset(DB::Specs::Interval& spec) const {
    const uint8_t* ptr = last_cell;
    size_t remain = cells.ptr - ptr;
    DB::Cells::Cell cell(&ptr, &remain);
    spec.offset_key.copy(cell.key);
    spec.offset_rev = cell.timestamp;
  }

  SWC_CAN_INLINE
  void move_last(InBlock* to) {
    const uint8_t* ptr = last_cell;
    size_t remain = cells.ptr - ptr;

    to->add(DB::Cells::Cell(&ptr, &remain));

    --header.cells_count;
    cells.ptr = const_cast<uint8_t*>(last_cell);
    cells.mark = nullptr;
  }

  void finalize_interval(bool any_begin, bool any_end) {
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    bool set_begin = true;

    DB::Cells::Cell cell;
    while(remain) {
      cell.read(&ptr, &remain);
      header.interval.align(cell.key);
      header.interval.expand(cell.timestamp);

      if(set_begin) {
        header.interval.expand_begin(cell);
        set_begin = false;
      }
    }
    header.interval.expand_end(cell);

    if(any_begin)
      header.is_any |= CellStore::Block::Header::ANY_BEGIN;
    if(any_end)
      header.is_any |= CellStore::Block::Header::ANY_END;
  }

  SWC_CAN_INLINE
  void finalize_encode(DB::Types::Encoder encoding) {
    header.encoder = encoding;
    DynamicBuffer output;
    CellStore::Block::Write::encode(err, cells, output, header);
    if(err)
      return;
    cells.free();
    cells.take_ownership(output);
  }

  DynamicBuffer             cells;
  CellStore::Block::Header  header;

  int                       err;

  private:

  const uint8_t*            last_cell;
};


CompactRange::CompactRange(Compaction* compactor, const RangePtr& range,
                           const uint32_t cs_size, const uint32_t blk_size)
            : ReqScan(
                ReqScan::Type::COMPACTION, true,
                compactor->cfg_read_ahead->get()/2, blk_size),
              compactor(compactor), range(range),
              cs_size(cs_size),
              blk_cells(range->cfg->block_cells()),
              blk_encoding(range->cfg->block_enc()),
              m_inblock(new InBlock(range->cfg->key_seq, blk_size)),
              m_processing(0),
              total_cells(0), total_blocks(0),
              time_intval(0), time_encode(0), time_write(0),
              state_default(Range::COMPACT_COMPACTING),
              req_last_time(0), m_stopped(false), m_chk_final(false),
              m_get(true), m_log_sz(0),
              m_chk_timer(
                asio::high_resolution_timer(Env::Rgr::io()->executor())) {
  spec.flags.max_versions = range->cfg->cell_versions();
}

CompactRange::~CompactRange() {
  if(m_inblock)
    delete m_inblock;

  InBlock* blk;
  while(m_q_intval.pop(&blk))
    delete blk;
  while(m_q_encode.pop(&blk))
    delete blk;
  while(m_q_write.pop(&blk))
    delete blk;
}

SWC_CAN_INLINE
CompactRange::Ptr CompactRange::shared() {
  return std::dynamic_pointer_cast<CompactRange>(shared_from_this());
}

void CompactRange::initialize() {
  // sync processing state
  range->compacting(Range::COMPACT_APPLYING);
  if(!range->blocks.wait_processing(Time::now_ns() + 60000000000)) {
    SWC_LOGF(LOG_WARN,
      "COMPACT-ERROR cancelled %lu/%lu waited 1-minute for processing",
      range->cfg->cid, range->rid);
    return compactor->compacted(shared(), range);
  }

  if(range->blocks.commitlog.cells_count(true))
    range->blocks.commitlog.commit_new_fragment(true);

  // ? immediate split
  if(range->blocks.cellstores.size() >= range->cfg->cellstore_max() * 2 &&
     range->blocks.cellstores.size_bytes(false) > cs_size) {
    size_t split_at = range->blocks.cellstores.size() / 2;
    auto it = range->blocks.cellstores.cbegin();

    split_option:
      it = range->blocks.cellstores.cbegin() + split_at;
      do {
        if(!(*it)->interval.key_begin.equal((*(it-1))->interval.key_end))
          break;
      } while(++it != range->blocks.cellstores.cend());

    if(it == range->blocks.cellstores.cend()) {
      if(split_at > 1) {
        split_at = 1;
        goto split_option;
      }
    } else {
      RangeSplit splitter(range, it - range->blocks.cellstores.cbegin());
      int err = splitter.run();
      if(!err)
        return finished(true);

      SWC_LOG_OUT(LOG_WARN,
        SWC_LOG_PRINTF("COMPACT-PROGRESS SPLIT RANGE cancelled %lu/%lu ",
                        range->cfg->cid, range->rid);
        Error::print(SWC_LOG_OSTREAM, err);
      );
    }
  }

  initial_commitlog(1);
}

void CompactRange::initial_commitlog(uint32_t tnum) {
  m_log_sz = range->blocks.commitlog.size();
  uint8_t cointervaling = range->cfg->log_compact_cointervaling();
  if(m_log_sz < cointervaling || m_stopped)
    return initial_commitlog_done(nullptr);

  CommitLog::Fragments::CompactGroups groups;
  if(range->blocks.commitlog.need_compact(groups, {}, cointervaling) &&
     !m_stopped) {
    new CommitLog::Compact(
      &range->blocks.commitlog, tnum, groups, cointervaling,
      [ptr=shared()] (const CommitLog::Compact* compact) {
        ptr->initial_commitlog_done(compact);
      }
    );
  } else {
    initial_commitlog_done(nullptr);
  }
}

void CompactRange::initial_commitlog_done(const CommitLog::Compact* compact) {
  if(m_stopped) {
    if(compact)
      delete compact;
    return;
  }
  if(compact) {
    uint32_t tnum = 0;
    uint8_t cointervaling = range->cfg->log_compact_cointervaling();
    if(compact->nfrags > 100 ||
       compact->ngroups > cointervaling ||
       compact->nfrags / compact->ngroups > cointervaling)
      tnum += compact->repetition + 1;
    delete compact;
    if(tnum) {
      range->compacting(Range::COMPACT_PREPARING); // range scan can continue
      return initial_commitlog(tnum);
    }
  }

  range->blocks.commitlog.get(fragments_old); // fragments for removal

  range->blocks.cellstores.get_key_end(m_required_key_last);
  for(auto& frag : fragments_old) {
    if(DB::KeySeq::compare(range->cfg->key_seq,
        m_required_key_last, frag->interval.key_end) == Condition::GT)
    m_required_key_last.copy(frag->interval.key_end);
  }
  SWC_LOG_OUT(LOG_INFO,
    SWC_LOG_PRINTF(
      "COMPACT-PROGRESS %lu/%lu early-split possible from scan offset ",
      range->cfg->cid, range->rid);
    m_required_key_last.print(SWC_LOG_OSTREAM);
  );

  range->compacting(state_default); // range scan &/ add can continue
  req_ts.store(Time::now_ns());
  progress_check_timer();

  range->scan_internal(shared());
}

bool CompactRange::with_block() const noexcept {
  return true;
}

bool CompactRange::selector(const DB::Types::KeySeq key_seq,
                            const DB::Cells::Cell& cell, bool&) {
  return spec.is_matching(
    key_seq, cell.key, cell.timestamp, cell.control & DB::Cells::TS_DESC)
    &&
    spec.key_intervals.is_matching_start(key_seq, cell.key);
}

bool CompactRange::reached_limits() {
  return m_stopped
      || (m_inblock->header.cells_count > 1 &&
          m_inblock->cells.fill() + m_inblock->cell_avg_size() >= blk_size)
      || m_inblock->header.cells_count >= blk_cells + 1;
}

bool CompactRange::add_cell_and_more(const DB::Cells::Cell& cell) {
  auto sz = m_inblock->cells.fill();
  m_inblock->add(cell);
  profile.add_cell(m_inblock->cells.fill() - sz);
  return !reached_limits();
}

void CompactRange::response(int& err) {
  if(m_stopped)
    return;

  total_cells.store(profile.cells_count);
  total_blocks.fetch_add(1);
  req_last_time.store(Time::now_ns() - req_ts);
  req_ts.store(Time::now_ns());

  profile.finished();
  SWC_LOG_OUT(LOG_INFO,
    SWC_LOG_PRINTF(
      "COMPACT-PROGRESS %lu/%lu blocks=%lu avg(i=%lu e=%lu w=%lu)us ",
      range->cfg->cid, range->rid,
      total_blocks.load(),
      (time_intval / total_blocks)/1000,
      (time_encode / total_blocks)/1000,
      (time_write / total_blocks)/1000
    );
    Error::print(SWC_LOG_OSTREAM, err);
    profile.print(SWC_LOG_OSTREAM << ' ');
  );

  if(err) {
    struct Task {
      Ptr ptr;
      SWC_CAN_INLINE
      Task(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
      void operator()() { ptr->quit(); }
    };
    Env::Rgr::maintenance_post(Task(shared()));
    return;
  }

  bool finishing;
  if((finishing = !reached_limits())) {
    stop_check_timer();
    state_default.store(Range::COMPACT_APPLYING);
    range->compacting(Range::COMPACT_APPLYING);
    range->blocks.wait_processing();
    range->blocks.commitlog.commit_new_fragment(true);
  }

  bool is_last;
  auto in_block = m_inblock->header.cells_count <= 1 ? nullptr : m_inblock;
  if(in_block) {
    m_processing.fetch_add(1);
    is_last = false;
    m_inblock = new InBlock(range->cfg->key_seq, blk_size, in_block);
    m_inblock->set_offset(spec);

    if(can_split_at() > 0 && DB::KeySeq::compare(range->cfg->key_seq,
        m_required_key_last, spec.offset_key) == Condition::GT) {
      if(spec.key_intervals.empty())
        spec.key_intervals.add();
      spec.key_intervals[0].start.set(spec.offset_key, Condition::EQ);
      spec.range_end.copy(spec.offset_key);
      spec.set_opt__key_equal();

      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_PRINTF(
          "COMPACT-PROGRESS %lu/%lu finishing early-split scan offset ",
          range->cfg->cid, range->rid
        );
        spec.offset_key.print(SWC_LOG_OSTREAM);
      );
    }
  } else {
    is_last = true;
    in_block = new InBlock(range->cfg->key_seq);
  }

  struct NextTask {
    Ptr ptr;
    SWC_CAN_INLINE
    NextTask(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
    void operator()() { ptr->process_interval(); }
  };
  if(m_q_intval.push_and_is_1st(in_block))
    Env::Rgr::maintenance_post(NextTask(shared()));

  if(m_stopped || is_last)
    return;

  finishing
    ? commitlog_done(nullptr)
    : commitlog(1);
}

bool CompactRange::is_slow_req(int64_t& median) const {
  median = (total_cells
    ? (Time::now_ns() - profile.ts_start) / total_cells : 10000)
    * blk_cells * 3;
  return req_last_time > median || Time::now_ns() - req_ts > median;
}

void CompactRange::commitlog(uint32_t tnum) {
  size_t log_sz = range->blocks.commitlog.size();
  uint8_t cointervaling = range->cfg->log_compact_cointervaling();
  if((log_sz > m_log_sz ? log_sz - m_log_sz : m_log_sz - log_sz)
      < cointervaling || m_stopped)
    return commitlog_done(nullptr);

  CommitLog::Fragments::CompactGroups groups;
  if(range->blocks.commitlog.need_compact(
      groups, fragments_old, cointervaling) && !m_stopped) {
    new CommitLog::Compact(
      &range->blocks.commitlog, tnum, groups, cointervaling,
      [ptr=shared()] (const CommitLog::Compact* compact) {
        ptr->commitlog_done(compact);
      }
    );
  } else {
    m_log_sz = log_sz;
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
    uint32_t tnum = 0;
    if(compact->nfrags > 100 ||
       compact->ngroups > range->cfg->log_rollout_ratio() ||
       compact->nfrags / compact->ngroups > range->cfg->log_rollout_ratio())
      tnum += compact->repetition + 1;
    delete compact;

    if(!m_stopped && !m_chk_final) {
      size_t bytes = range->blocks.commitlog.size_bytes_encoded();
      float fits = float(bytes)/cs_size;
      if(size_t(fits) + 1 >= range->cfg->cellstore_max() &&
         fits - size_t(fits) >= float(range->cfg->compact_percent())/100) {
        stop_check_timer();
        state_default.store(Range::COMPACT_PREPARING);
        range->compacting(Range::COMPACT_PREPARING);
        SWC_LOGF(LOG_INFO,
          "COMPACT-MITIGATE(add req.) %lu/%lu reached max-log-size(%lu)",
          range->cfg->cid, range->rid, bytes);
      } else {
        int64_t median;
        range->compacting(tnum && is_slow_req(median)
          ? Range::COMPACT_PREPARING : state_default.load());
      }
      if(tnum && range->is_loaded())
        return commitlog(tnum);
    }
  }

  m_get.stop();
  request_more();
}

void CompactRange::progress_check_timer() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_stopped || m_chk_final)
      return;
  }

  int64_t median;
  bool slow = is_slow_req(median);
  if(range->compacting_ifnot_applying(
      slow ? Range::COMPACT_PREPARING : state_default.load())) {
    request_more();
  }
  if((median /= 1000000) < 1000)
    median = 1000;

  Core::MutexSptd::scope lock(m_mutex);
  if(m_stopped || m_chk_final)
    return;
  m_chk_timer.expires_after(std::chrono::milliseconds(median));
  struct TimerTask {
    Ptr ptr;
    SWC_CAN_INLINE
    TimerTask(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
    void operator()(const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted)
        ptr->progress_check_timer();
    }
  };
  m_chk_timer.async_wait(TimerTask(shared()));
}

void CompactRange::stop_check_timer() {
  bool at = false;
  if(m_chk_final.compare_exchange_weak(at, true)) {
    Core::MutexSptd::scope lock(m_mutex);
    m_chk_timer.cancel();
  }
}

void CompactRange::request_more() {
  if(m_get || m_stopped)
    return;

  size_t sz = m_processing;
  if(sz && (sz >= compactor->cfg_read_ahead->get() ||
      (sz > Env::Rgr::res().avail_ram()/blk_size &&
       range->blocks.release(sz * blk_size) < sz * blk_size))) {
    return;
  }

  struct Task {
    Ptr ptr;
    SWC_CAN_INLINE
    Task(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
    void operator()() { ptr->range->scan_internal(ptr->get_req_scan()); }
  };
  if(!m_get.running())
    Env::Rgr::maintenance_post(Task(shared()));
}

SWC_CAN_INLINE
void CompactRange::process_interval() {
  struct NextTask {
    Ptr ptr;
    SWC_CAN_INLINE
    NextTask(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
    void operator()() { ptr->process_encode(); }
  };
  Time::Measure_ns t_measure;
  _do: {
    auto in_block(m_q_intval.front());
    if(!in_block->is_last) {
      request_more();
      in_block->finalize_interval(false, false);
    }
    bool more = m_q_intval.pop_and_more() && !m_stopped;
    in_block->_other = nullptr;
    if(m_q_encode.push_and_is_1st(in_block))
      Env::Rgr::maintenance_post(NextTask(shared()));
    if(more)
      goto _do;
  }
  time_intval.fetch_add(t_measure.elapsed());
  request_more();
}

SWC_CAN_INLINE
void CompactRange::process_encode() {
  struct NextTask {
    Ptr ptr;
    SWC_CAN_INLINE
    NextTask(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
    void operator()() { ptr->process_write(); }
  };
  Time::Measure_ns t_measure;
  _do: {
    auto in_block(m_q_encode.front());
    if(!in_block->is_last) {
      request_more();
      in_block->finalize_encode(blk_encoding);
    }
    bool more = m_q_encode.pop_and_more() && !m_stopped;
    in_block->_other = nullptr;
    if(m_q_write.push_and_is_1st(in_block))
      Env::Rgr::maintenance_post(NextTask(shared()));
    if(more)
      goto _do;
  }
  time_encode.fetch_add(t_measure.elapsed());
  request_more();
}

SWC_CAN_INLINE
void CompactRange::process_write() {
  Time::Measure_ns t_measure;
  int err = Error::OK;
  _do: {
    auto in_block(m_q_write.front());
    if(in_block->is_last) {
      time_write.fetch_add(t_measure.elapsed());
      return finalize();
    }

    if(in_block->err)
      return quit();

    write_cells(err, in_block);

    if(err || !range->is_loaded() || compactor->stopped())
      return quit();

    bool more = m_q_write.pop_and_more() && !m_stopped;
    delete in_block;

    m_processing.fetch_sub(1);
    request_more();
    if(more)
      goto _do;
  }

  time_write.fetch_add(t_measure.elapsed());
  request_more();
}

csid_t CompactRange::create_cs(int& err) {
  if(!tmp_dir) {
    err = Error::OK;
    auto cs_tmp_dir = range->get_path(Range::CELLSTORES_TMP_DIR);
    if(Env::FsInterface::interface()->exists(err, cs_tmp_dir) && !err)
      Env::FsInterface::interface()->rmdir(err, cs_tmp_dir);
    if(!err)
      Env::FsInterface::interface()->mkdirs(err, cs_tmp_dir);
    if(err)
      return 0;
    tmp_dir = true;
  }

  csid_t csid = 1;
  {
    Core::MutexSptd::scope lock(m_mutex);
    csid += cellstores.size();
  }
  cs_writer.reset(new CellStore::Write(
    csid,
    range->get_path_cs_on(Range::CELLSTORES_TMP_DIR, csid),
    range,
    spec.flags.max_versions
  ));
  cs_writer->create(err, -1, range->cfg->file_replication(), blk_size);

  if(!m_chk_final) {
    uint32_t p = range->cfg->compact_percent()/10;
    if(csid == range->cfg->cellstore_max() * (p ? p : 1)) {
      stop_check_timer();
      state_default.store(Range::COMPACT_PREPARING);
      if(range->compacting_ifnot_applying(Range::COMPACT_PREPARING)) {
        SWC_LOGF(LOG_INFO,
          "COMPACT-MITIGATE(add req.) %lu/%lu reached cs-max(%u)",
          range->cfg->cid, range->rid, csid);
      }
    }
  }
  return csid;

}

SWC_CAN_INLINE
void CompactRange::write_cells(int& err, InBlock* in_block) {
  if(!cs_writer) {
    if(create_cs(err) == 1 && range->_is_any_begin())
      in_block->header.is_any |= CellStore::Block::Header::ANY_BEGIN;
    if(err)
      return;
  }

  cs_writer->block_write(err, in_block->cells, std::move(in_block->header));
  if(err)
    return;

  if(cs_writer->size >= cs_size ||
     cs_writer->blocks_count() == CellStore::Read::MAX_BLOCKS - 1) {
    add_cs(err);
    if(err)
      return;
  }
}

void CompactRange::add_cs(int& err) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    cs_writer->prev_key_end.copy(
      cellstores.empty()
        ? range->prev_range_end
        : cellstores.back()->interval.key_end
    );
  }
  cs_writer->finalize(err);
  {
    Core::MutexSptd::scope lock(m_mutex);
    cellstores.push_back(cs_writer);
  }
  cs_writer = nullptr;
}

ssize_t CompactRange::can_split_at() {
  auto max = range->cfg->cellstore_max();

  Core::MutexSptd::scope lock(m_mutex);
  if(cellstores.size() < (max < 2 ? 2 : max))
    return 0;

  auto it = cellstores.cbegin() + 1;
  if((*it)->size < (cs_size/100) * range->cfg->compact_percent())
    return -2;

  size_t at = cellstores.size() / 2;

  split_option:
    it = cellstores.cbegin() + at;
    do {
      if(!(*it)->interval.key_begin.equal((*(it-1))->interval.key_end))
        break;
    } while(++it != cellstores.cend());

  if(it == cellstores.cend()) {
    if(at > 1) {
      at = 1;
      goto split_option;
    }
    return -1;
  }
  return it - cellstores.cbegin();
}

void CompactRange::finalize() {
  if(m_stopped)
    return;
  stop_check_timer();

  if(!range->is_loaded())
    return quit();

  Time::Measure_ns t_measure;
  int err = Error::OK;
  bool empty_cs = false;

  if(m_inblock->header.cells_count) {
    // first or/and last block of any-type set with empty-key
    bool any_begin = false;
    if(!cs_writer) {
      any_begin = create_cs(err) == 1 && range->_is_any_begin();
      if(err)
        return quit();
    }
    m_inblock->finalize_interval(any_begin, range->_is_any_end());
    cs_writer->block_encode(
      err, m_inblock->cells, std::move(m_inblock->header));

  } else if(!cellstores.size() && !cs_writer) {
    // as an initial empty range cs with range intervals
    empty_cs = true;
    create_cs(err); //csid_t csid =
    if(err)
      return quit();
    range->_get_interval(
      m_inblock->header.interval.key_begin,
      m_inblock->header.interval.key_end
    );
    if(m_inblock->header.interval.key_begin.empty())
      m_inblock->header.is_any |= CellStore::Block::Header::ANY_BEGIN;
    if(m_inblock->header.interval.key_end.empty())
      m_inblock->header.is_any |= CellStore::Block::Header::ANY_END;
    cs_writer->block_encode(
      err, m_inblock->cells, std::move(m_inblock->header));
  }
  if(err)
    return quit();

  if(cs_writer) {
    if(!cs_writer->size)
      return quit();
    add_cs(err);
    if(err)
      return quit();
  }
  time_write.fetch_add(t_measure.elapsed());

  range->compacting(Range::COMPACT_APPLYING);
  range->blocks.wait_processing();

  ssize_t split_at = can_split_at();
  if(split_at == -1) {
    SWC_LOGF(LOG_WARN,
      "COMPACT-SPLIT %lu/%lu fail(versions-over-cs) cs-count=%ld",
      range->cfg->cid, range->rid, int64_t(cellstores.size()));

  } else if(split_at == -2) {
    SWC_LOGF(LOG_DEBUG,
      "COMPACT-SPLIT %lu/%lu skipping(last-cs-small) cs-count=%ld",
      range->cfg->cid, range->rid, int64_t(cellstores.size()));
  }

  split_at > 0
    ? mngr_create_range(split_at)
    : apply_new(empty_cs);

}

void CompactRange::mngr_create_range(uint32_t split_at) {
  struct ReqData {
    CompactRange::Ptr ptr;
    uint32_t          split_at;
    SWC_CAN_INLINE
    ReqData(const CompactRange::Ptr& ptr, uint32_t split_at)
            noexcept : ptr(ptr), split_at(split_at) {
    }
    SWC_CAN_INLINE
    cid_t get_cid() const noexcept {
      return ptr->range->cfg->cid;
    }
    SWC_CAN_INLINE
    client::Clients::Ptr& get_clients() noexcept {
      return Env::Clients::get();
    }
    SWC_CAN_INLINE
    bool valid() noexcept {
      return !ptr->m_stopped && !Env::Rgr::is_not_accepting();
    }
    SWC_CAN_INLINE
    void callback(
        const Comm::client::ConnQueue::ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RangeCreateRsp& rsp) {
      SWC_LOGF(LOG_DEBUG,
        "Compact::Mngr::Req::RangeCreate err=%d(%s) %lu/%lu",
        rsp.err, Error::get_text(rsp.err), get_cid(), rsp.rid);

      if(rsp.err && valid() &&
         rsp.err != Error::CLIENT_STOPPING &&
         rsp.err != Error::COLUMN_NOT_EXISTS &&
         rsp.err != Error::COLUMN_MARKED_REMOVED &&
         rsp.err != Error::COLUMN_NOT_READY) {
        req->request_again();
        return;
      }
      if((!rsp.err || rsp.err == Error::COLUMN_NOT_READY) && rsp.rid)
        ptr->split(rsp.rid, split_at);
      else
        ptr->apply_new();
    }
  };
  Comm::Protocol::Mngr::Req::RangeCreate<ReqData>::request(
    Comm::Protocol::Mngr::Params::RangeCreateReq(
      range->cfg->cid, Env::Rgr::rgr_data()->rgrid),
    10000,
    shared(), split_at
  );
}


template<bool clear>
struct CompactRange::TaskFinished {
  Ptr ptr;
  SWC_CAN_INLINE
  TaskFinished(Ptr&& ptr) noexcept : ptr(std::move(ptr)) { }
  void operator()() { ptr->finished(clear); }
};


void CompactRange::split(rid_t new_rid, uint32_t split_at) {
  ColumnPtr col = Env::Rgr::columns()->get_column(range->cfg->cid);
  if(!col || col->removing())
    return quit();

  Time::Measure_ns t_measure;
  SWC_LOGF(LOG_INFO, "COMPACT-SPLIT %lu/%lu new-rid=%lu",
           range->cfg->cid, range->rid, new_rid);

  int err = Error::OK;
  auto new_range = col->internal_create(err, new_rid, true);
  if(!err)
    new_range->internal_create_folders(err);
  if(err) {
    SWC_LOGF(LOG_INFO, "COMPACT-SPLIT cancelled err=%d %lu/%lu new-rid=%lu",
            err, range->cfg->cid, range->rid, new_rid);
    err = Error::OK;
    new_range->compacting(Range::COMPACT_NONE);
    col->internal_remove(err, new_rid);
    mngr_remove_range(new_range);
    return apply_new();
  }

  CellStore::Writers new_cellstores;
  auto it = cellstores.cbegin() + split_at;
  new_cellstores.assign(it, cellstores.cend());
  cellstores.erase(it, cellstores.cend());

  new_range->internal_create(err, new_cellstores);
  if(!err)
    range->apply_new(err, cellstores, fragments_old);

  if(err) {
    err = Error::OK;
    new_range->compacting(Range::COMPACT_NONE);
    col->internal_remove(err, new_rid);
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

  new_range->expand_and_align(false, Query::Update::CommonMeta::make(
    new_range,
    [col, ptr=shared()] (const Query::Update::CommonMeta::Ptr& hdlr) {
      SWC_LOGF(LOG_INFO,
        "COMPACT-SPLIT %lu/%lu unloading new-rid=%lu reg-err=%d(%s)",
          col->cfg->cid, ptr->range->rid, hdlr->range->rid,
          hdlr->error(), Error::get_text(hdlr->error()));
      hdlr->range->compacting(Range::COMPACT_NONE);
      col->internal_unload(hdlr->range->rid);

      struct ReqData  {
        CompactRange::Ptr ptr;
        const rid_t       new_rid;
        SWC_CAN_INLINE
        ReqData(const CompactRange::Ptr& ptr, rid_t new_rid)
                noexcept : ptr(ptr), new_rid(new_rid) {
        }
        SWC_CAN_INLINE
        cid_t get_cid() const noexcept {
          return ptr->range->cfg->cid;
        }
        SWC_CAN_INLINE
        client::Clients::Ptr& get_clients() noexcept {
          return Env::Clients::get();
        }
        SWC_CAN_INLINE
        bool valid() noexcept {
          return !Env::Rgr::is_not_accepting();
        }
        SWC_CAN_INLINE
        void callback(
            const Comm::client::ConnQueue::ReqBase::Ptr& req,
            const Comm::Protocol::Mngr::Params::RangeUnloadedRsp& rsp) {
          SWC_LOGF(LOG_DEBUG,
            "Compact::Mngr::Req::RangeUnloaded err=%d(%s) %lu/%lu",
            rsp.err, Error::get_text(rsp.err), get_cid(), new_rid);
          if(rsp.err && valid() &&
             rsp.err != Error::CLIENT_STOPPING &&
             rsp.err != Error::COLUMN_NOT_EXISTS &&
             rsp.err != Error::COLUMN_MARKED_REMOVED &&
             rsp.err != Error::COLUMN_NOT_READY) {
            req->request_again();
          }
        }
      };
      Comm::Protocol::Mngr::Req::RangeUnloaded<ReqData>::request(
        Comm::Protocol::Mngr::Params::RangeUnloadedReq(
          hdlr->range->cfg->cid, hdlr->range->rid),
        10000,
        ptr, hdlr->range->rid
      );
  }));

  range->expand_and_align(true, Query::Update::CommonMeta::make(
    range,
    [t_measure, ptr=shared()]
    (const Query::Update::CommonMeta::Ptr&) mutable {
      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_PRINTF("COMPACT-SPLITTED %lu/%lu took=%luns new-end=",
          ptr->range->cfg->cid, ptr->range->rid, t_measure.elapsed());
          ptr->cellstores.back()->interval.key_end.print(SWC_LOG_OSTREAM);
      );
      Env::Rgr::maintenance_post(TaskFinished<true>(std::move(ptr)));
    }
  ));
}

void CompactRange::apply_new(bool clear) {
  int err = Error::OK;
  Query::Update::CommonMeta::Cb_t cb;
  if(clear) {
    cb = [ptr=shared()] (const Query::Update::CommonMeta::Ptr&) mutable {
      Env::Rgr::maintenance_post(TaskFinished<true>(std::move(ptr)));
    };
  } else {
    cb = [ptr=shared()] (const Query::Update::CommonMeta::Ptr&) mutable {
      Env::Rgr::maintenance_post(TaskFinished<false>(std::move(ptr)));
    };
  }
  range->apply_new(
    err, cellstores, fragments_old,
    Query::Update::CommonMeta::make(range, std::move(cb))
  );
  if(err)
    return quit();
}

bool CompactRange::completion() {
  bool at = false;
  if(!m_stopped.compare_exchange_weak(at, true))
    return false;

  stop_check_timer();

  auto ptr = shared();
  for(int chk = 0; ptr.use_count() > 3; ++chk) { // insure sane
    if(chk == 3000) {
      SWC_LOGF(LOG_INFO, "COMPACT-STOPPING %lu/%lu use_count=%ld",
                range->cfg->cid, range->rid, ptr.use_count());
      chk = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return true;
}

void CompactRange::finished(bool clear) {
  bool ok = completion();
  profile.finished();

  SWC_LOG_OUT(LOG_INFO,
    SWC_LOG_PRINTF(
      "COMPACT-FINISHED %lu/%lu cells=%lu blocks=%lu "
      "(total=%ld intval=%lu encode=%lu write=%lu)ms ",
      range->cfg->cid, range->rid, total_cells.load(), total_blocks.load(),
      (Time::now_ns() - profile.ts_start)/1000000,
      time_intval/1000000, time_encode/1000000, time_write/1000000
    );
    profile.print(SWC_LOG_OSTREAM);
  );

  range->compact_require(false);
  if(ok)
    compactor->compacted(shared(), range, clear);
}

void CompactRange::quit() {
  if(!completion())
    return;

  int err = Error::OK;
  if(cs_writer) {
    cs_writer->remove(err);
    cs_writer = nullptr;
  }
  if(tmp_dir) {
    tmp_dir = false;
    Env::FsInterface::interface()->rmdir(
      err, range->get_path(Range::CELLSTORES_TMP_DIR));
  }
  SWC_LOGF(LOG_INFO, "COMPACT-ERROR cancelled %lu/%lu",
           range->cfg->cid, range->rid);

  compactor->compacted(shared(), range);
}






}}
