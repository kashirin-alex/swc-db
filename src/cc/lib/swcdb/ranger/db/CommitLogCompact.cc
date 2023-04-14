/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

SWC_CAN_INLINE
Compact::Group::Group(Compact* a_compact, uint8_t a_worker)
                      : ts(), read_frags(),
                        worker(a_worker), error(Error::OK),
                        compact(a_compact),
                        m_idx(0), m_running(0), m_finishing(0),
                        m_mutex(),
                        m_cells(
                          compact->log->range->cfg->key_seq,
                          compact->log->range->cfg->block_cells() * 2,
                          compact->log->range->cfg->cell_versions(),
                          compact->log->range->cfg->cell_ttl(),
                          compact->log->range->cfg->column_type()
                        ),
                        m_remove(), m_fragments() {
}

void Compact::Group::run(bool initial) {
  size_t running;
  if(initial) {
    running = compact->preload;
    m_finishing.store(read_frags.size() + 1);
  } else {
    running = m_running.fetch_sub(1);
  }

  if(running == compact->preload) do {
    size_t idx = m_idx.fetch_add(1);
    if(idx >= read_frags.size())
      break;
    if(error || compact->log->stopping) {
      m_finishing.fetch_sub(read_frags.size() - idx);
      break;
    }
    running = m_running.add_rslt(1);
    read_frags[idx]->processing_increment();
    read_frags[idx]->load(this);
  } while(running < compact->preload);

  if(m_finishing.fetch_sub(1) == 1)
    write();
}

void Compact::Group::loaded(Fragment::Ptr&& frag) {
  struct Task {
    Group*        g;
    Fragment::Ptr frag;
    SWC_CAN_INLINE
    Task(Group* a_g, Fragment::Ptr&& a_frag) noexcept
        : g(a_g), frag(std::move(a_frag)) { }
    SWC_CAN_INLINE
    Task(Task&& other) noexcept
        : g(other.g), frag(std::move(other.frag)) { }
    Task(const Task&) = delete;
    Task& operator=(Task&&) = delete;
    Task& operator=(const Task&) = delete;
    ~Task() noexcept { }
    void operator()() { g->_loaded(frag); }
  };
  Env::Rgr::post(Task(this, std::move(frag)));
}

void Compact::Group::_loaded(const Fragment::Ptr& frag) {
  if(compact->log->stopping || error) {
    frag->processing_decrement();
    return run(false);
  }

  int err;
  if(!frag->loaded(err)) {
    SWC_LOG_OUT(LOG_WARN,
      Error::print(
        SWC_LOG_OSTREAM << "COMPACT-LOG fragment retrying to ", err);
      frag->print(SWC_LOG_OSTREAM << ' ');
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    frag->load(this);
    return;
  }

  Env::Rgr::res().more_mem_future(frag->size_bytes());
  {
    Core::MutexSptd::scope lock(m_mutex);
    frag->load_cells(err, m_cells);
    m_remove.push_back(frag);
  }
  frag->release();
  Env::Rgr::res().less_mem_future(frag->size_bytes());
  run(false);
}

void Compact::Group::write() {
  Core::Semaphore sem(5);
  size_t total_cells_count = 0;
  int err;
  uint32_t cells_count;
  if(compact->log->stopping || error || m_cells.empty())
    goto finished_write;

  do {
    DynamicBuffer cells;
    DB::Cells::Interval interval(m_cells.key_seq);
    StaticBuffer::Ptr buff_write(new StaticBuffer());
    m_cells.write_and_free(
      cells, cells_count = 0, interval,
      compact->log->range->cfg->block_size(),
      compact->log->range->cfg->block_cells()
    );
    total_cells_count += cells_count;

    auto frag = Fragment::make_write(
      err = Error::OK,
      compact->get_filepath(compact->log->next_id()),
      std::move(interval),
      compact->log->range->cfg->block_enc(),
      compact->log->range->cfg->cell_versions(),
      cells_count, cells,
      buff_write
    );
    if(err)
      error.store(err);
    if(!frag)
      break;
    m_fragments.push_back(frag);

    buff_write->own = false;
    sem.acquire();
    frag->write(
      Error::UNPOSSIBLE,
      compact->log->range->cfg->file_replication(),
      buff_write,
      &sem
    );
  } while(!error && !m_cells.empty());

  finished_write:
    sem.wait_all();
    compact->finished(this, total_cells_count);
}

void Compact::Group::finalize() {
  int err = Error::OK;
  if(!error && !compact->log->stopping &&
     read_frags.size() == m_remove.size()) {
    compact->log->take_ownership(err, m_fragments, m_remove);
  }

  if(!m_fragments.empty()) {
    Core::Semaphore sem(10);
    for(auto frag : m_fragments) {
      sem.acquire();
      frag->remove(err = Error::OK, &sem);
    }
    sem.wait_all();
  }

  struct Task {
    Compact* compact;
    SWC_CAN_INLINE
    Task(Compact* a_compact) noexcept : compact(a_compact) { }
    void operator()() { compact->finalized(); }
  };
  Env::Rgr::post(Task(compact));
}



Compact::Compact(Fragments* a_log, uint32_t a_repetition,
                 const Fragments::CompactGroups& groups,
                 uint8_t cointervaling,
                 Compact::Cb_t&& cb)
                : log(a_log), ts(),
                  preload(log->range->cfg->log_fragment_preload()),
                  repetition(a_repetition),
                  ngroups(groups.size()), nfrags(0),
                  m_workers(), m_groups(),
                  m_cb(std::move(cb)) {
  for(auto frags : groups)
    nfrags += frags.size();

  uint32_t blks = Env::Rgr::res().avail_ram() / log->range->cfg->block_size();
  if(blks < nfrags)
    log->range->blocks.release((nfrags-blks) * log->range->cfg->block_size());
  if(!blks)
    blks = log->range->cfg->log_rollout_ratio();

  size_t g_sz = Env::Rgr::res().concurrency() / 2;
  m_groups.reserve(g_sz + 1);

  for(auto frags : groups) {
    if(frags.empty())
      continue;
    m_groups.push_back(new Group(this, m_groups.size()+1));

    for(auto it = frags.cbegin(); it != frags.cend(); ++it) {
      m_groups.back()->read_frags.push_back(*it);
      if(!blks) {
        if(m_groups.back()->read_frags.size() < cointervaling)
          continue;
        break;
      }
      --blks;
    }
    if(!blks || m_groups.size() >= g_sz)
      break;
  }

  if(m_groups.empty()) {
    m_cb ? m_cb(this) : log->finish_compact(this);
    return;
  }

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-START " SWC_FMT_LU "/" SWC_FMT_LU " w=" SWC_FMT_LD
    " frags=" SWC_FMT_LU "(" SWC_FMT_LU ")/" SWC_FMT_LU " repetition=%u",
    log->range->cfg->cid, log->range->rid,
    int64_t(m_groups.size()), nfrags, ngroups, log->size(), repetition
  );

  m_workers.store(m_groups.size());

  std::sort(m_groups.begin(), m_groups.end(),
    [](const Group* p1, const Group* p2) {
      return p1->read_frags.size() >= p2->read_frags.size(); });

  for(auto g : m_groups)
    g->run(true);
}

void Compact::finished(Group* group, size_t cells_count) {
  size_t running(m_workers.sub_rslt(1));

  auto took = group->ts.elapsed();
  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-PROGRESS " SWC_FMT_LU "/" SWC_FMT_LU
    " running=" SWC_FMT_LU
    " worker=%u " SWC_FMT_LU "us cells=" SWC_FMT_LU "(" SWC_FMT_LU "ns)",
    log->range->cfg->cid, log->range->rid, running,
    group->worker, took/1000,
    cells_count, cells_count ? took/cells_count: 0
  );
  if(running)
    return;

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-FINISHING " SWC_FMT_LU "/" SWC_FMT_LU" w=" SWC_FMT_LD,
    log->range->cfg->cid, log->range->rid, int64_t(m_groups.size()));

  log->range->compacting(Range::COMPACT_APPLYING);
  log->range->blocks.wait_processing(); // sync processing state

  m_workers.store(m_groups.size());

  struct Task {
    Group* g;
    SWC_CAN_INLINE
    Task(Group* a_g) noexcept : g(a_g) { }
    void operator()() {
      g->finalize();
      delete g;
    }
  };
  for(auto g : m_groups)
    Env::Rgr::post(Task(g));
}

void Compact::finalized() {
  if(m_workers.fetch_sub(1) != 1)
    return;

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-FINISH " SWC_FMT_LU "/" SWC_FMT_LU " w=" SWC_FMT_LD
    " frags=" SWC_FMT_LU "(" SWC_FMT_LU ")/" SWC_FMT_LU
    " repetition=%u " SWC_FMT_LU "ns",
    log->range->cfg->cid, log->range->rid,
    int64_t(m_groups.size()), nfrags, ngroups, log->size(), repetition,
    ts.elapsed()
  );

  m_cb ? m_cb(this) : log->finish_compact(this);
}

SWC_CAN_INLINE
std::string Compact::get_filepath(const int64_t frag) const {
  std::string s(log->range->get_path(Range::LOG_TMP_DIR));
  std::string tmp(std::to_string(frag));
  s.reserve(s.length() + 6 + tmp.length());
  s.append("/");
  s.append(tmp);
  s.append(".frag");
  return s;
}



}}} // namespace SWC::Ranger::CommitLog

