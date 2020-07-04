/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/ranger/db/Compaction.h"
#include "swcdb/ranger/db/CompactRange.h"


namespace SWC { namespace Ranger {


Compaction::Compaction() 
          : m_check_timer(
              asio::high_resolution_timer(
                *RangerEnv::maintenance_io()->ptr())),
            m_run(true), m_running(0), m_scheduled(false),
            m_idx_cid(0), m_idx_rid(0), 
            cfg_read_ahead(
              Env::Config::settings()->get<Property::V_GUINT8>(
                "swc.rgr.compaction.read.ahead")),
            cfg_max_range(
              Env::Config::settings()->get<Property::V_GUINT8>(
                "swc.rgr.compaction.range.max")), 
            cfg_check_interval(
              Env::Config::settings()->get<Property::V_GINT32>(
                "swc.rgr.compaction.check.interval")) {
}

Compaction::~Compaction() { }

bool Compaction::available() {
  std::scoped_lock lock(m_mutex);
  return m_running < cfg_max_range->get();
}

void Compaction::stop() {
  std::unique_lock lock_wait(m_mutex);
  m_run = false;
  m_check_timer.cancel();
  if(m_running) 
    m_cv.wait(lock_wait, [&running=m_running](){return !running;});  
}

void Compaction::schedule() {
  schedule(cfg_check_interval->get());
}

void Compaction::schedule(uint32_t t_ms) {
  std::scoped_lock lock(m_mutex);
  _schedule(t_ms);
}

bool Compaction::stopped() {
  std::scoped_lock lock(m_mutex);
  return !m_run;
}

void Compaction::run(bool continuing) {
  {
    std::scoped_lock lock(m_mutex); 
    if(!m_run || (!continuing && m_scheduled))
      return;
    m_scheduled = true;
  }

  RangePtr range  = nullptr;
  for(Column::Ptr col = nullptr; 
      !stopped() && 
      (col || (col = RangerEnv::columns()->get_next(m_idx_cid)) ); ) {

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

    {
      std::scoped_lock lock(m_mutex); 
      ++m_running;
    }
    asio::post(*RangerEnv::maintenance_io()->ptr(), 
      [this, range](){ compact(range); } );
    
    if(!available())
      return;
  }
  
  {
    std::scoped_lock lock(m_mutex); 
    if(m_running)
      return;
  }
  compacted();
}

void Compaction::compact(const RangePtr& range) {

  if(!range->is_loaded() || stopped())
    return compacted(range);

  auto& commitlog  = range->blocks.commitlog;

  uint32_t cs_size = range->cfg->cellstore_size(); 
  uint32_t blk_size = range->cfg->block_size();
  uint8_t perc = range->cfg->compact_percent(); 
  uint32_t allow_sz = (cs_size  / 100) * perc; 
  uint32_t cell_revs = range->cfg->cell_versions();
  uint64_t cell_ttl = range->cfg->cell_ttl();

  size_t value;
  bool do_compaction = false;
  std::string need;

  if(do_compaction = range->compact_required() && commitlog.cells_count()) {
    need.append("Required");

  } else if(do_compaction = (value = commitlog.size_bytes(true)) >= allow_sz) {
    need.append("LogBytes=");
    need.append(std::to_string(value-allow_sz));

  } else if(do_compaction = (value = commitlog.size()) > cs_size/blk_size) {
    need.append("LogCount=");
    need.append(std::to_string(value-cs_size/blk_size));

  } else if(do_compaction = range->blocks.cellstores.need_compaction(
                    cs_size + allow_sz,  blk_size + (blk_size / 100) * perc)) {
    need.append("CsResize");

  } else if(do_compaction = cell_ttl && 
            (int64_t)(value = range->blocks.cellstores.get_ts_earliest())
            != DB::Cells::AUTO_ASSIGN && 
            (int64_t)value < Time::now_ns()-cell_ttl*100) {
    need.append("CsTTL");

  } else if(do_compaction = range->blocks.cellstores.get_cell_revs() 
                              > cell_revs) {
    need.append("CsVersions");
  }

  if(stopped() || !do_compaction)
    return compacted(range);
    
  SWC_LOGF(LOG_INFO, "COMPACT-STARTED %lu/%lu %s", 
           range->cfg->cid, range->rid, need.c_str());

  auto req = std::make_shared<CompactRange>(
    this,
    range, 
    cs_size,
    blk_size
  );
  req->initialize();
}


void Compaction::compacted(const RangePtr& range, bool all) {
  if(all) {
    range->blocks.release(0);
    if(range->blocks.size())
      SWC_LOGF(LOG_ERROR, "%s", range->to_string().c_str());
    SWC_ASSERT(!range->blocks.size());

  } else if(size_t bytes = Env::Resources.need_ram()) {
    range->blocks.release(bytes);
  }

  range->compacting(Range::COMPACT_NONE);
  compacted();
}

void Compaction::compacted() {
  std::scoped_lock lock(m_mutex);

  if(m_running && m_running-- == cfg_max_range->get()) {
    asio::post(*RangerEnv::maintenance_io()->ptr(), [this](){ run(true); });
    return;
  }
  if(m_run) {
    if(!m_running) {
      m_scheduled = false;
      _schedule(cfg_check_interval->get());
    }
    return;
  }
  m_cv.notify_all();
}

void Compaction::_schedule(uint32_t t_ms) {
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

  SWC_LOGF(LOG_DEBUG, "Ranger compaction scheduled in ms=%u", t_ms);
}




}}