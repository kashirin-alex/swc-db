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
            cfg_check_interval(
              Env::Config::settings()->get<Config::Property::V_GINT32>(
                "swc.rgr.compaction.check.interval")),
            m_check_timer(
              asio::high_resolution_timer(
                *Env::Rgr::maintenance_io()->ptr())),
            m_run(true), m_running(0), m_scheduled(false),
            m_idx_cid(0), m_idx_rid(0)  {
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
      !stopped() && !Env::Rgr::res().is_low_mem_state() &&
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

    {
      std::scoped_lock lock(m_mutex); 
      ++m_running;
    }
    Env::Rgr::maintenance_post([this, range](){ compact(range); } );
    
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
    return compacted(nullptr, range);

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
             (int64_t)(value = range->blocks.cellstores.get_ts_earliest())
                != DB::Cells::AUTO_ASSIGN && 
             value < Time::now_ns()-cell_ttl*100)) {
    need.append("CsTTL");

  } else if((do_compaction = range->blocks.cellstores.get_cell_revs() 
                              > cell_revs)) {
    need.append("CsVersions");
  }

  if(stopped() || !do_compaction)
    return compacted(nullptr, range);
    
  SWC_LOGF(LOG_INFO, "COMPACT-STARTED %lu/%lu %s", 
           range->cfg->cid, range->rid, need.c_str());

  auto req = std::make_shared<CompactRange>(
    this,
    range, 
    cs_size,
    blk_size
  );
  {
    std::scoped_lock lock(m_mutex); 
    m_compacting.push_back(req);
  }
  req->initialize();
}


void Compaction::compacted(const CompactRange::Ptr req,
                           const RangePtr& range, bool all) {
  if(all) {
    range->blocks.release(0);
    if(range->blocks.size())
      SWC_LOG_OUT(LOG_ERROR, range->print(SWC_LOG_OSTREAM, false); );
    SWC_ASSERT(!range->blocks.size());

  } else if(size_t bytes = Env::Rgr::res().need_ram()) {
    range->blocks.release(bytes);
  }

  range->compacting(Range::COMPACT_NONE);
  compacted();
  
  if(req) {
    std::scoped_lock lock(m_mutex); 
    auto it = std::find(m_compacting.begin(), m_compacting.end(), req);
    if(it != m_compacting.end())
      m_compacting.erase(it);
  }
}

void Compaction::compacted() {
  std::scoped_lock lock(m_mutex);

  if(m_running && m_running-- == cfg_max_range->get()) {
    Env::Rgr::maintenance_post([this](){ run(true); });
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
}




}}