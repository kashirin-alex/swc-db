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
                          compact->log->range->cfg->block_cells() * 2,
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
  bool finished;
  do {
    frag = m_queue.front();

    err = Error::OK;
    if(!compact->log->stopping && !error) {
      frag->load_cells(err, m_cells);
      m_remove.push_back(frag);
    } else {
      frag->processing_decrement();
    }
    frag->release();
    if(err)
      SWC_LOGF(LOG_ERROR, 
        "COMPACT-LOG-ERROR %d/%d err=%d(%s) %s", 
        compact->log->range->cfg->cid, compact->log->range->rid,
        err, Error::get_text(err), frag->to_string().c_str()
      );

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
  size_t total_cells_count = 0;
  int err = Error::OK;
  do {
    DynamicBuffer cells;
    uint32_t cells_count = 0;
    DB::Cells::Interval interval(m_cells.key_seq);
    auto buff_write = std::make_shared<StaticBuffer>();

    m_cells.write_and_free(
      cells, cells_count, interval, 
      compact->log->range->cfg->block_size(), 
      compact->log->range->cfg->block_cells()
    );
    total_cells_count += cells_count;

    auto frag = Fragment::make_write(
      err, 
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

  if(!compact->log->stopping && !error)
    compact->log->remove(err, m_remove);
  else if(!tmp_frags.empty())
    compact->log->remove(err, tmp_frags);
}


Compact::Compact(Fragments* log, int repetition, 
                 const std::vector<Fragments::Vec>& groups, Cb_t& cb)
                : log(log), ts(Time::now_ns()),
                  repetition(repetition), ngroups(groups.size()), nfrags(0), 
                  m_cb(cb) {
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
        if(m_groups.back()->read_frags.size() < Fragments::MIN_COMPACT)
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
    "COMPACT-LOG-START %d/%d w=%lld frags=%lld(%lld)/%lld repetition=%d",
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
    "COMPACT-LOG-PROGRESS %d/%d running=%d "
    "worker=%d %lldus cells=%lld(%lldns)",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns() - group->ts)/1000,
    cells_count, cells_count ? (Time::now_ns() - group->ts)/cells_count: 0
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
    "COMPACT-LOG-FINISH %d/%d w=%lld frags=%lld(%lld)/%lld"
    " repetition=%d %lldns",
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

