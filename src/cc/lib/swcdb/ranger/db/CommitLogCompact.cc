/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

Compact::Group::Group(Compact* compact, uint8_t worker) 
                      : ts(Time::now_ns()), worker(worker), error(Error::OK), 
                        compact(compact),
                        m_idx(0), m_running(0), m_finishing(0),
                        m_cells(
                          compact->log->range->cfg->key_seq,
                          compact->log->range->cfg->block_cells() * 2,
                          compact->log->range->cfg->cell_versions(), 
                          compact->log->range->cfg->cell_ttl(), 
                          compact->log->range->cfg->column_type()
                        ) {
}

Compact::Group::~Group() {
  if(!m_cells.empty())
    Env::Rgr::res().less_mem_usage(m_cells.size_of_internal());
}

void Compact::Group::run(bool initial) {
  size_t running;
  if(initial) {
    running = compact->preload;
    m_finishing = read_frags.size() + 1;
  } else {
    running = m_running.fetch_sub(1, std::memory_order_relaxed);
  }

  if(running == compact->preload) do {
    size_t idx = m_idx.fetch_add(1, std::memory_order_relaxed);
    if(idx >= read_frags.size())
      break;
    if(error || compact->log->stopping) {
      m_finishing -= read_frags.size() - idx;
      break;
    }
    running = m_running.fetch_add(1, std::memory_order_relaxed) + 1;
    read_frags[idx]->load([this] (const Fragment::Ptr& frag) {
      Env::Rgr::post([this, frag]() { loaded(frag); });
    });
  } while(running < compact->preload);

  if(m_finishing.fetch_sub(1, std::memory_order_relaxed) == 1)
    write();
}

void Compact::Group::loaded(const Fragment::Ptr& frag) {
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
    frag->load([this] (const Fragment::Ptr& frag) {
      Env::Rgr::post([this, frag]() { loaded(frag); });
    });
    frag->processing_decrement();
    return;
  }

  ssize_t adj_sz;
  {
    Core::MutexSptd::scope lock(m_mutex);
    size_t sz = m_cells.size_of_internal();
    frag->load_cells(err, m_cells);
    adj_sz = ssize_t(m_cells.size_of_internal()) - sz;
    m_remove.push_back(frag);
  }
  Env::Rgr::res().adj_mem_usage(adj_sz);
  frag->release();
  run(false);
}

void Compact::Group::write() {
  Core::Semaphore sem(5);
  size_t total_cells_count = 0;
  int err;
  ssize_t sz;
  uint32_t cells_count;
  if(compact->log->stopping || error || m_cells.empty())
    goto finished_write;

  do {
    DynamicBuffer cells;
    DB::Cells::Interval interval(m_cells.key_seq);
    auto buff_write = std::make_shared<StaticBuffer>();
    sz = m_cells.size_of_internal();
    m_cells.write_and_free(
      cells, cells_count = 0, interval, 
      compact->log->range->cfg->block_size(), 
      compact->log->range->cfg->block_cells()
    );
    Env::Rgr::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);
    total_cells_count += cells_count;

    auto frag = Fragment::make_write(
      err = Error::OK, 
      compact->get_filepath(compact->log->next_id()),
      interval, 
      compact->log->range->cfg->block_enc(), 
      compact->log->range->cfg->cell_versions(),
      cells_count, cells, 
      buff_write
    );
    if(err)
      error = err;
    if(!frag)
      break;
    m_fragments.push_back(frag);

    buff_write->own = false;
    sem.acquire();
    frag->write(
      Error::UNPOSSIBLE, 
      compact->log->range->cfg->file_replication(), 
      frag->offset_data + frag->size_enc, 
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

  compact->finalized();
}



Compact::Compact(Fragments* log, int repetition,
                 const std::vector<Fragments::Vec>& groups,
                 uint8_t cointervaling,
                 Cb_t& cb)
                : log(log), ts(Time::now_ns()),
                  preload(log->range->cfg->log_fragment_preload()),
                  repetition(repetition), ngroups(groups.size()), nfrags(0), 
                  m_cb(cb) {
  for(auto frags : groups)
    nfrags += frags.size();
    
  uint32_t blks = Env::Rgr::res().avail_ram() / log->range->cfg->block_size();
  if(blks < nfrags)
    log->range->blocks.release((nfrags-blks) * log->range->cfg->block_size());
  if(!blks)
    blks = log->range->cfg->log_rollout_ratio();

  for(auto frags : groups) {
    if(frags.empty())
      continue;
    m_groups.push_back(new Group(this, m_groups.size()+1));

    for(auto it = frags.begin(); it < frags.end(); ++it) {
      m_groups.back()->read_frags.push_back(*it);
      if(!blks) {
        if(m_groups.back()->read_frags.size() < cointervaling)
          continue;
        break;
      }
      --blks;
    }
    if(!blks || m_groups.size() >= Env::Rgr::res().concurrency()/2 )
      break;
  }

  if(m_groups.empty()) {
    if(m_cb)
      m_cb(this);
    else
      log->finish_compact(this);
    return;
  }

  SWC_LOGF(LOG_INFO, 
    "COMPACT-LOG-START %lu/%lu w=%lu frags=%lu(%lu)/%lu repetition=%d",
    log->range->cfg->cid, log->range->rid, 
    m_groups.size(), nfrags, ngroups, log->size(), repetition
  );

  m_workers = m_groups.size();

  std::sort(m_groups.begin(), m_groups.end(), 
    [](const Group* p1, const Group* p2) {
      return p1->read_frags.size() >= p2->read_frags.size(); });

  for(auto g : m_groups)
    g->run(true);
}

Compact::~Compact() { }

void Compact::finished(Group* group, size_t cells_count) {
  size_t running = m_workers.fetch_sub(1, std::memory_order_relaxed);
  --running;

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-PROGRESS %lu/%lu running=%lu "
    "worker=%u %ldus cells=%lu(%ldns)",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns() - group->ts)/1000,
    cells_count, cells_count ? (Time::now_ns() - group->ts)/cells_count: 0
  );
  if(running)
    return;

  SWC_LOGF(LOG_INFO, "COMPACT-LOG-FINISHING %lu/%lu w=%ld",
                      log->range->cfg->cid, log->range->rid, m_groups.size());

  log->range->compacting(Range::COMPACT_APPLYING);
  log->range->blocks.wait_processing(); // sync processing state
  
  m_workers = m_groups.size();
  for(auto g : m_groups)
    Env::Rgr::post([g]() { g->finalize(); });
}

void Compact::finalized() {
  if(m_workers.fetch_sub(1, std::memory_order_relaxed) > 1)
    return;

  for(auto g : m_groups)
    delete g;

  auto took = Time::now_ns() - ts;
  SWC_LOGF(LOG_INFO, 
    "COMPACT-LOG-FINISH %lu/%lu w=%ld frags=%ld(%ld)/%ld"
    " repetition=%ld %ldns",
    log->range->cfg->cid, log->range->rid, 
    m_groups.size(), nfrags, ngroups, log->size(), repetition, took
  );
  if(m_cb)
    m_cb(this);
  else
    log->finish_compact(this);
}

const std::string Compact::get_filepath(const int64_t frag) const {
  std::string s(log->range->get_path(Range::LOG_TMP_DIR));
  s.append("/");
  s.append(std::to_string(frag));
  s.append(".frag");
  return s;
}



}}} // namespace SWC::Ranger::CommitLog

