/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/ranger/db/Compaction.h"


namespace SWC { namespace Ranger {


Compaction::Compaction()
          : cfg_read_ahead(
              Env::Config::settings()->get<Config::Property::V_GUINT8>(
                "swc.rgr.compaction.read.ahead")),
            cfg_max_range(
              Env::Config::settings()->get<Config::Property::V_GUINT8>(
                "swc.rgr.compaction.range.max")),
            cfg_max_log(
              Env::Config::settings()->get<Config::Property::V_GUINT8>(
                "swc.rgr.compaction.commitlog.max")),
            cfg_check_interval(
              Env::Config::settings()->get<Config::Property::V_GINT32>(
                "swc.rgr.compaction.check.interval")),
            m_run(true), m_running(0), m_log_compactions(0),
            m_check_timer(
              asio::high_resolution_timer(
                Env::Rgr::maintenance_io()->executor())),
            m_idx_cid(0), m_idx_rid(0)  {
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
  return m_running < cfg_max_range->get();
}

void Compaction::stop() {
  m_run.store(false);
  uint8_t sz = 0;
  for(uint8_t n=0; m_running; ++n) {
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
  if(m_running || m_schedule.running())
    m_cv.wait(lock_wait, [this](){
      return !m_running && !m_schedule.running();});
  m_schedule.stop();
}

SWC_CAN_INLINE
void Compaction::schedule() {
  schedule(cfg_check_interval->get());
}

SWC_CAN_INLINE
void Compaction::schedule(uint32_t t_ms) {
  Core::ScopedLock lock(m_mutex);
  _schedule(t_ms);
}

SWC_CAN_INLINE
bool Compaction::stopped() {
  return !m_run;
}

void Compaction::run(bool initial) {
  if(stopped() || m_running == cfg_max_range->get() || m_schedule.running())
    return;

  uint8_t added = 0;
  RangePtr range  = nullptr;
  for(ColumnPtr col = nullptr;
      !stopped() && (!m_running || !Env::Rgr::res().is_low_mem_state()) &&
      (col || (col = Env::Rgr::columns()->get_next(m_idx_cid)) ); ) {

    if(col->removing()) {
      ++m_idx_cid;
      col = nullptr;
      continue;
    }

    if(!(range = col->get_next(m_idx_rid))) {
      ++m_idx_cid;
      col = nullptr;
      continue;
    }
    ++m_idx_rid;

    if((!range->compact_required() && range->blocks.commitlog.try_compact()) ||
        !range->compact_possible())
      continue;

    if(uint8_t running = compact(range)) {
      ++added;
      if(running == cfg_max_range->get())
        break;
    }
  }

  m_schedule.stop();
  if(initial && !added && !stopped())
    schedule();
}

uint8_t Compaction::compact(const RangePtr& range) {

  if(!range->is_loaded() || stopped()) {
    range->compacting(Range::COMPACT_NONE);
    return 0;
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

  if((do_compaction = range->compact_required() && commitlog.cells_count())) {
    need.append("Required");

  } else if((do_compaction = (value = commitlog.size_bytes(true))
                                                    >= allow_sz)) {
    need.append("LogBytes=");
    need.append(std::to_string(value-allow_sz));

  } else if((do_compaction = (value = commitlog.size()) > cs_size/blk_size)) {
    need.append("LogCount=");
    need.append(std::to_string(value-cs_size/blk_size));

  } else if((do_compaction = range->blocks.cellstores.size() > cs_max ||
                             range->blocks.cellstores.need_compaction(
                cs_size + allow_sz,  blk_size + (blk_size / 100) * perc) )) {
    need.append("CsResize");

  } else if((do_compaction = cell_ttl &&
             int64_t(value = range->blocks.cellstores.get_ts_earliest())
                != DB::Cells::AUTO_ASSIGN &&
             value < Time::now_ns()-cell_ttl*100)) {
    need.append("CsTTL");

  } else if((do_compaction = range->blocks.cellstores.get_cell_revs()
                              > cell_revs)) {
    need.append("CsVersions");
  }

  if(stopped() || !do_compaction) {
    range->compacting(Range::COMPACT_NONE);
    return 0;
  }

  SWC_LOGF(LOG_INFO, "COMPACT-STARTED %lu/%lu %s",
           range->cfg->cid, range->rid, need.c_str());

  CompactRange::Ptr req(new CompactRange(this, range, cs_size, blk_size));
  {
    Core::ScopedLock lock(m_mutex);
    m_compacting.push_back(req);
  }
  uint8_t running = m_running.add_rslt(1);
  Env::Rgr::maintenance_post([req](){ req->initialize(); } );
  return running;
}


void Compaction::compacted(const CompactRange::Ptr req,
                           const RangePtr& range, bool all) {
  if(all) {
    range->blocks.reset_blocks();

  } else if(size_t bytes = Env::Rgr::res().need_ram()) {
    range->blocks.release(bytes);
  }

  range->compacting(Range::COMPACT_NONE);
  compacted();

  Core::ScopedLock lock(m_mutex);
  auto it = std::find(m_compacting.begin(), m_compacting.end(), req);
  if(it != m_compacting.end())
    m_compacting.erase(it);
}

SWC_CAN_INLINE
void Compaction::compacted() {
  uint8_t ran = m_running.fetch_sub(1);
  if(ran == cfg_max_range->get()) {
    Env::Rgr::maintenance_post([this](){ run(false); });

  } else if(ran == 1) {
    if(stopped()) {
      Core::ScopedLock lock(m_mutex);
      m_cv.notify_all();
    } else {
      schedule();
    }
  }
}

void Compaction::_schedule(uint32_t t_ms) {
  if(stopped() || m_schedule)
    return;

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_check_timer.expiry();
  auto now = asio::high_resolution_timer::clock_type::now();
  if(set_on > now && set_on < now + set_in)
    return;

  m_check_timer.expires_after(set_in);
  m_check_timer.async_wait(
    [this](const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted){
        run();
      }
  });

  SWC_LOGF(LOG_DEBUG, "Ranger compaction scheduled in ms=%u", t_ms);
  return;
}




}}