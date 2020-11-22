/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

Compact::Group::Group(Compact* compact, uint8_t worker) 
                      : ts(Time::now_ns()), worker(worker), error(Error::OK), 
                        compact(compact),
                        m_sem(4),
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

void Compact::Group::run() {
  Env::Rgr::post([this]() { load(); });
}

void Compact::Group::load() {
  for(auto frag : read_frags) {
    if(error || compact->log->stopping)
      break;
    m_sem.acquire();
    frag->load([this, frag] () { loaded(frag); });
  }
  m_sem.wait_all();
  write();
}

void Compact::Group::loaded(Fragment::Ptr frag) {
  if(compact->log->stopping || error) {
    frag->processing_decrement();
    m_sem.release();
    return;
  }

  int err;
  if(!frag->loaded(err)) {
    frag->processing_decrement();
    SWC_LOG_OUT(LOG_WARN,
      Error::print(
        SWC_LOG_OSTREAM << "COMPACT-LOG fragment retrying to ", err);
      frag->print(SWC_LOG_OSTREAM << ' ');
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    frag->load([this, frag]() { loaded(frag); } );
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
  m_sem.release();
}

void Compact::Group::write() {
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
    m_sem.acquire();
    frag->write(
      Error::UNPOSSIBLE, 
      compact->log->range->cfg->file_replication(), 
      frag->offset_data + frag->size_enc, 
      buff_write,
      &m_sem
    );
  } while(!error && !m_cells.empty());

  finished_write:
    m_sem.wait_all();
    compact->finished(this, total_cells_count);
}

void Compact::Group::finalize() {
  if(!error && !compact->log->stopping) {
    if(read_frags.size() != m_remove.size())
      error = Error::CANCELLED;
  }

  Fragments::Vec tmp_frags;
  int err = Error::OK;
  for(auto frag : m_fragments) {
    if(compact->log->stopping || error) {
      frag->remove(err = Error::OK);
    } else {
      auto tmp = compact->log->take_ownership(err = Error::OK, frag);
      if(tmp) {
        tmp_frags.push_back(tmp);
      } else {
        if(err)
          error = err;
        frag->remove(err = Error::OK);
      }
    }
    if(err)
      error = err;
    delete frag;
  }

  if(compact->log->stopping || error) {
    if(!tmp_frags.empty())
      compact->log->remove(err, tmp_frags);
  } else {
    compact->log->remove(err, m_remove);
  }
}



Compact::Compact(Fragments* log, int repetition, 
                 const std::vector<Fragments::Vec>& groups, Cb_t& cb)
                : log(log), ts(Time::now_ns()),
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
        if(m_groups.back()->read_frags.size() < Fragments::MIN_COMPACT)
          continue;
        break;
      }
      --blks;
    }
    if(!blks || m_groups.size() >= Env::Rgr::res().concurrency()/2 )
      break;
  }
  
  m_workers = m_groups.size();
  
  SWC_LOGF(LOG_INFO, 
    "COMPACT-LOG-START %lu/%lu w=%lu frags=%lu(%lu)/%lu repetition=%d",
    log->range->cfg->cid, log->range->rid, 
    m_groups.size(), nfrags, ngroups, log->size(), repetition
  );

  for(auto g : m_groups)
    g->run();
}

Compact::~Compact() { }

void Compact::finished(Group* group, size_t cells_count) {
  m_mutex.lock();
  uint8_t running = --m_workers;
  m_mutex.unlock();

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-PROGRESS %lu/%lu running=%d "
    "worker=%u %ldus cells=%lu(%ldns)",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns() - group->ts)/1000,
    cells_count, cells_count ? (Time::now_ns() - group->ts)/cells_count: 0
  );
  if(running)
    return;

  log->range->compacting(Range::COMPACT_APPLYING);
  log->range->blocks.wait_processing(); // sync processing state
  
  Core::Semaphore sem(m_groups.size(), m_groups.size());
  for(auto g : m_groups) {
    Env::Rgr::post([g, &sem]() { 
      g->finalize();
      delete g;
      sem.release();
    });
  }
  sem.wait_all();

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

