/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

Compact::Group::Group(Compact* compact, uint8_t worker) 
                      : ts(Time::now_ns()), worker(worker), error(Error::OK), 
                        compact(compact), m_read_idx(0), 
                        m_loading(0), m_processed(0),
                        m_cells(
                          compact->log->range->cfg->key_seq,
                          compact->log->range->cfg->block_cells()*2,
                          compact->log->range->cfg->cell_versions(), 
                          compact->log->range->cfg->cell_ttl(), 
                          compact->log->range->cfg->column_type()
                        ), 
                        m_sem(5) {
}

Compact::Group::Group::~Group() { }

void Compact::Group::run() {
  asio::post(*Env::IoCtx::io()->ptr(), [this]() { load_more(); });
}

void Compact::Group::load_more() {
  Fragment::Ptr frag;
  bool more;
  do {
    m_mutex.lock();
    if(error || compact->log->stopping) {
      more = false;
      m_read_idx = read_frags.size();
    } else if(more = m_read_idx < read_frags.size() &&
                     m_loading < Fragments::MAX_PRELOAD) {
      ++m_loading;
      frag = read_frags[m_read_idx];
      ++m_read_idx;
    }
    m_mutex.unlock();
    if(more)
      frag->load([this, frag] () { loaded(frag); });
  }while(more);
}

void Compact::Group::loaded(Fragment::Ptr frag) {
  if(m_queue.push_and_is_1st(frag))
    asio::post(*Env::IoCtx::io()->ptr(), [this]() { load(); });
}

void Compact::Group::load() {
  int err;
  Fragment::Ptr frag;
  uint64_t ts;
  bool finished;
  do {
    frag = m_queue.front();

    ts = Time::now_ns();
    if(frag->loaded(err = Error::OK) && !compact->log->stopping && !error) {
      frag->load_cells(err, m_cells);
      m_remove.push_back(frag);
    }
    frag->processing_decrement();
    frag->release();
    if(err)
      SWC_LOGF(LOG_ERROR, 
        "COMPACT-LOG-ERROR %d/%d err=%d(%s) %s", 
        compact->log->range->cfg->cid, compact->log->range->rid,
        err, Error::get_text(err), frag->to_string().c_str()
      );
    if(!compact->log->stopping && !error) {
      ts = Time::now_ns() - ts;
      SWC_LOGF(LOG_INFO, 
        "COMPACT-LOG-PROGRESS %d/%d w=%d(%lld/%lld) %lld(%lldns/cell)"
        " sz=%lld begin-%s", 
        compact->log->range->cfg->cid, compact->log->range->rid, 
        worker, m_remove.size(), read_frags.size(), 
        frag->cells_count, frag->cells_count? ts/frag->cells_count: 0, 
        m_cells.size(), frag->interval.key_begin.to_string().c_str()
      );
    }

    m_mutex.lock();
    finished = (!--m_loading && m_queue.size() == 1 && 
                (error || compact->log->stopping) ) || 
               ++m_processed == read_frags.size();
    m_mutex.unlock();

    if(finished)
      return write();
    if(!error && !compact->log->stopping)
      load_more();

  } while(m_queue.pop_and_more());
}

void Compact::Group::write() {
  if(compact->log->stopping || error || m_cells.empty())
    return;
  
  int err = Error::OK;
  do {
    DynamicBuffer cells;
    auto frag = Fragment::make(
      compact->get_filepath(Time::now_ns()), 
      m_cells.key_seq, 
      Fragment::State::WRITING
    );
    m_fragments.push_back(frag);

    m_cells.write_and_free(
      cells, 
      frag->cells_count, frag->interval, 
      compact->log->range->cfg->block_size(), 
      compact->log->range->cfg->block_cells()
    );

    frag->write(
      err, 
      compact->log->range->cfg->file_replication(), 
      compact->log->range->cfg->block_enc(), 
      cells, 
      compact->log->range->cfg->cell_versions(),
      &m_sem
    );
    if(err)
      error = err;

    m_sem.wait_until_under(5);
  } while(!error && !m_cells.empty());

  m_sem.wait_all();
  compact->finished(this);
}

void Compact::Group::finalize() {
  int err = Error::OK;
  for(auto frag : m_fragments) {
    if(compact->log->stopping || error)
      frag->remove(err);
    else
      compact->log->take_ownership(err, frag); 
      // tmp_took_frags (remove on err)
    if(err)
      error = err;
    delete frag;
  }

  if(!compact->log->stopping && !error)
    compact->log->remove(err, m_remove);
  //else + tmp_took_frags
}


Compact::Compact(Fragments* log, int repetition, 
                 const std::vector<std::vector<Fragment::Ptr>>& groups,
                 Cb_t& cb)
                : log(log), ts(Time::now_ns()),
                  repetition(repetition), nfrags(0), m_cb(cb) {
  for(auto frags : groups)
    nfrags += frags.size();
    
  uint32_t blks = (Env::Resources.avail_ram() /log->range->cfg->block_size());
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
        if(m_groups.back()->read_frags.size() < 2)
          continue;
        break;
      }
      --blks;
    }
    if(!blks)
      break;
  }
  
  m_workers = m_groups.size();
  
  SWC_LOGF(LOG_INFO, 
    "COMPACT-LOG-START %d/%d frags=%lld(%lld)/%lld repetition=%d",
    log->range->cfg->cid, log->range->rid, 
    nfrags, m_groups.size(), log->size(), repetition
  );

  for(auto g : m_groups)
    g->run();
}

Compact::~Compact() { }

void Compact::finished(Group* group) {
  m_mutex.lock();
  uint8_t running = --m_workers;
  m_mutex.unlock();

  SWC_LOGF(LOG_INFO,
    "COMPACT-LOG-PROGRESS %d/%d running=%d worker=%d-%lldns",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns()-group->ts) 
  );
  if(running)
    return;

  log->range->compacting(Range::COMPACT_APPLYING);
  log->range->blocks.wait_processing(); // sync processing state
  for(auto g : m_groups) {
    g->finalize();
    delete g;
  }

  auto took = Time::now_ns() - ts;
  SWC_LOGF(LOG_INFO, 
    "COMPACT-LOG-FINISH %d/%d frags=%lld(%lld)/%lld repetition=%d %lldns",
    log->range->cfg->cid, log->range->rid, 
    nfrags, m_groups.size(), log->size(), repetition, took
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

