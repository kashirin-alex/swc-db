/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/ranger/db/Compaction.h"


namespace SWC { namespace Ranger {


Compaction::Compaction()
          : cfg_read_ahead(
              Env::Config::settings()->get<Config::Property::Value_uint8_g>(
                "swc.rgr.compaction.read.ahead")),
            cfg_max_range(
              Env::Config::settings()->get<Config::Property::Value_uint8_g>(
                "swc.rgr.compaction.range.max")),
            cfg_max_log(
              Env::Config::settings()->get<Config::Property::Value_uint8_g>(
                "swc.rgr.compaction.commitlog.max")),
            cfg_check_interval(
              Env::Config::settings()->get<Config::Property::Value_int32_g>(
                "swc.rgr.compaction.check.interval")),
            cfg_uncompacted_max(
              Env::Config::settings()->get<Config::Property::Value_int32_g>(
                "swc.rgr.compaction.range.uncompacted.max")),
            m_run(true),
            m_schedule(), m_running(0),
            m_log_chk(), m_log_compactions(0),
            m_mutex(),
            m_check_timer(
              asio::high_resolution_timer(
                Env::Rgr::maintenance_io()->executor())),
            m_cv(),
            m_last_cid(0), m_idx_cid(0),
            m_last_rid(0), m_idx_rid(0),
            m_next(false), m_uncompacted(0),
            m_compacting() {
}

SWC_CAN_INLINE
bool Compaction::log_compact_possible() noexcept {
  if(m_log_chk.running())
    return false;
  bool ok = uint16_t(m_log_compactions) + m_running
            < uint16_t(cfg_max_log->get());
  if(ok)
    m_log_compactions.fetch_add(1);
  m_log_chk.stop();
  return ok;
}

SWC_CAN_INLINE
void Compaction::log_compact_finished() noexcept {
  m_log_compactions.fetch_sub(1);
}

SWC_CAN_INLINE
bool Compaction::available() noexcept {
  return m_running < cfg_max_range->get() &&
         (!m_running || !Env::Rgr::res().is_low_mem_state());
}

void Compaction::stop() {
  m_run.store(false);
  {
    Core::UniqueLock lock_wait(m_mutex);
    if(m_schedule.running())
      m_cv.wait(lock_wait, [this](){ return !m_schedule.running(); });
    m_check_timer.cancel();
  }
  uint8_t sz = 0;
  for(uint8_t n=0; ; ++n) {
    CompactRange::Ptr req;
    {
      Core::ScopedLock lock(m_mutex);
      if(m_compacting.empty())
        break;
      if(sz && sz == m_compacting.size()) {
        if(n >= m_compacting.size())
          break;
      } else {
        sz = m_compacting.size();
        n = 0;
      }
      req = m_compacting[n];
    }
    req->quit();
  }
  Core::UniqueLock lock_wait(m_mutex);
  m_check_timer.cancel();
  if(m_running)
    m_cv.wait(lock_wait, [this](){ return !m_running; });
  m_schedule.stop();
}

SWC_CAN_INLINE
bool Compaction::stopped() {
  return !m_run;
}

void Compaction::run() {
  for(ColumnPtr col = nullptr;
      !stopped() && available() &&
      (col || (col=Env::Rgr::columns()->get_next(m_last_cid, m_idx_cid)));) {

    RangePtr range;
    if(col->removing() ||
       !(range = col->get_next(m_last_rid, m_idx_rid))) {
      ++m_idx_cid;
      col = nullptr;
      m_last_rid = 0;
      m_idx_rid = 0;
      continue;
    }
    ++m_idx_rid;

    SWC_LOGF(LOG_DEBUG, "COMPACT-CHECKING " SWC_FMT_LU "/" SWC_FMT_LU,
                          range->cfg->cid, range->rid);

    if((!range->compact_required() && range->blocks.commitlog.try_compact()) ||
        !range->compact_possible())
      continue;

    compact(range);
  }
  m_next.store(m_idx_cid);
  if(!m_idx_cid)
    m_uncompacted = 0;
  m_schedule.stop();

  if(stopped()) {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  } else {
    schedule(cfg_check_interval->get());
  }
}

void Compaction::compact(const RangePtr& range) {

  if(!range->is_loaded() || stopped()) {
    range->compacting(Range::COMPACT_NONE);
    return;
  }

  auto& commitlog  = range->blocks.commitlog;

  uint32_t cs_size = range->cfg->cellstore_size();
  size_t cs_max = range->cfg->cellstore_max();

  uint32_t blk_size = range->cfg->block_size();
  if(cs_size < blk_size)
    blk_size = cs_size;
  uint8_t perc = range->cfg->compact_percent();
  uint32_t allow_sz = (cs_size  / 100) * perc;
  uint32_t cell_revs = range->cfg->cell_versions();
  uint64_t cell_ttl = range->cfg->cell_ttl();

  size_t value;
  bool do_compaction = false;
  std::string need;

  if((do_compaction = range->compact_required() && !commitlog.empty())) {
    need.append("Required");

  } else if((do_compaction = (value = commitlog.size_bytes(true))
                                                    >= allow_sz)) {
    need.append("LogBytes=");
    need.append(std::to_string(value-allow_sz));

  } else if((do_compaction = (value = commitlog.size()) > cs_size/blk_size)) {
    need.append("LogCount=");
    need.append(std::to_string(value-cs_size/blk_size));

  } else if((do_compaction = range->blocks.cellstores.need_compaction(
              cs_max, cs_size+allow_sz, blk_size+(blk_size/100)*perc) )) {
    need.append("CsResize");

  } else if((do_compaction = cell_ttl &&
             int64_t(value = range->blocks.cellstores.get_ts_earliest())
                != DB::Cells::TIMESTAMP_AUTO &&
             value < Time::now_ns()-cell_ttl*100)) {
    need.append("CsTTL");

  } else if((do_compaction = range->blocks.cellstores.get_cell_revs()
                              > cell_revs)) {
    need.append("CsVersions");

  } else if((do_compaction =
        (Time::now_ns() - commitlog.modification_ts())/1000000
          > cfg_check_interval->get() &&
        !commitlog.empty() &&
        ++m_uncompacted > size_t(cfg_uncompacted_max->get()))) {
    need.append("Uncompacted=");
    need.append(std::to_string(m_uncompacted--));
  }

  if(stopped() || !do_compaction) {
    range->compacting(Range::COMPACT_NONE);
    return;
  }

  Env::Rgr::res().more_mem_future(blk_size);
  m_running.fetch_add(1);

  SWC_LOGF(LOG_INFO, "COMPACT-STARTED " SWC_FMT_LU "/" SWC_FMT_LU " %s",
           range->cfg->cid, range->rid, need.c_str());

  struct Task {
    CompactRange::Ptr req;
    SWC_CAN_INLINE
    Task(CompactRange* a_req) noexcept : req(a_req) { }
    SWC_CAN_INLINE
    Task(Task&& other) noexcept : req(std::move(other.req)) { }
    Task(const Task&) = delete;
    Task& operator=(Task&&) = delete;
    Task& operator=(const Task&) = delete;
    ~Task() noexcept { }
    void operator()() { req->initialize(); }
  };

  Task task(new CompactRange(this, range, cs_size, blk_size));
  {
    Core::ScopedLock lock(m_mutex);
    m_compacting.push_back(task.req);
  }
  Env::Rgr::maintenance_post(std::move(task));
}

void Compaction::compacted(const CompactRange::Ptr req,
                           const RangePtr& range, bool all) {
  if(all) {
    range->blocks.reset_blocks();

  } else if(size_t bytes = Env::Rgr::res().need_ram()) {
    range->blocks.release(bytes);
  }

  range->compacting(Range::COMPACT_NONE);
  bool ok;
  {
    Core::ScopedLock lock(m_mutex);
    auto it = std::find(m_compacting.cbegin(), m_compacting.cend(), req);
    if((ok = it != m_compacting.cend()))
      m_compacting.erase(it);
  }
  if(ok) {
    Env::Rgr::res().less_mem_future(req->blk_size);
    compacted();
  } else {
    SWC_LOGF(LOG_WARN,
      "Ranger compaction track missed(" SWC_FMT_LU "/" SWC_FMT_LU ")",
      range->cfg->cid, range->rid);
  }
}

SWC_CAN_INLINE
void Compaction::compacted() {
  if(m_running.fetch_sub(1) == 1 && stopped()) {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  } else {
    schedule(cfg_check_interval->get());
  }
}

SWC_SHOULD_NOT_INLINE
void Compaction::schedule(uint32_t t_ms) {
  if(stopped() || !available())
    return;

  if(t_ms < 3 || m_next) {
    if(!m_schedule.running()) {
      struct Task {
        Compaction* ptr;
        SWC_CAN_INLINE
        Task(Compaction* a_ptr) noexcept : ptr(a_ptr) { }
        void operator()() { ptr->run(); }
      };
      Env::Rgr::maintenance_post(Task(this));
    }
    return;
  }

  {
    auto set_in = std::chrono::milliseconds(t_ms);
    auto now = asio::high_resolution_timer::clock_type::now();
    Core::ScopedLock lock(m_mutex);
    auto set_on = m_check_timer.expiry();
    if(set_on > now) {
      if(set_on < now + set_in)
        return;
      m_check_timer.cancel();
    }
    m_check_timer.expires_after(set_in);
    struct TimerTask {
      Compaction* ptr;
      SWC_CAN_INLINE
      TimerTask(Compaction* a_ptr) noexcept : ptr(a_ptr) { }
      void operator()(const asio::error_code& ec) {
        if(ec != asio::error::operation_aborted)
          ptr->schedule(0);
      }
    };
    m_check_timer.async_wait(TimerTask(this));
  }
  SWC_LOGF(LOG_DEBUG, "Ranger compaction scheduled in ms=%u", t_ms);
}



}}
