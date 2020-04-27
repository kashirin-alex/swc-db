/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

Compact::Group::Group(Compact* compact, uint8_t worker) 
                      : ts(Time::now_ns()), worker(worker), error(Error::OK), 
                        compact(compact), m_cells(compact->key_seq), 
                        m_remain(0), m_sem(5) {
}

Compact::Group::Group::~Group() { }

void Compact::Group::run(const std::vector<Fragment::Ptr>& frags) {
  m_cells.reset(
    compact->log->range->cfg->cell_versions(), 
    compact->log->range->cfg->cell_ttl(), 
    compact->log->range->cfg->column_type()
  );
  m_remain += frags.size();

  Fragment::Ptr frag;
  for(auto it = frags.begin(); it<frags.end(); ++it) {
    if(m_queue.size() == Fragments::MAX_PRELOAD) {
      std::unique_lock lock_wait(m_mutex);
      m_cv.wait(lock_wait, [this]() { 
        return m_queue.size() < Fragments::MAX_PRELOAD; 
      });
    }
    if(error || compact->log->stopping)
      break;

    m_queue.push(frag = *it);
    (*it)->load([this]() { loaded(); });
  }
  
  std::unique_lock lock_wait(m_mutex);
  if(!m_queue.empty())
    m_cv.wait(lock_wait, [this]() { 
      loaded();
      return m_queue.empty(); 
    });
  
  write(true);
  finalize();
}

void Compact::Group::loaded() {
  if(m_queue.activating())
    asio::post(*Env::IoCtx::io()->ptr(), [this](){ load(); });
}

void Compact::Group::load() {
  bool isloaded;
  int err = Error::OK;
  Fragment::Ptr frag;
  uint64_t ts;
  do {
    err = Error::OK;
    if(compact->log->stopping || error) {
      do {
        (frag = m_queue.front())->processing_decrement();
        frag->release();
      } while(!m_queue.deactivating());
      break;
    }

    if(isloaded = (frag = m_queue.front())->loaded(err)) {
      ts = Time::now_ns();
      frag->load_cells(err, m_cells); 
      m_remove.push_back(frag);
      --m_remain;

      ts = Time::now_ns() - ts;
      SWC_LOGF(LOG_INFO, 
        "COMPACT-FRAGMENTS-PROGRESS %d/%d worker=%d remain=%lld "
        "cells=%lld avg=%lldns compacting=%lld begin-%s", 
        compact->log->range->cfg->cid, compact->log->range->rid, worker, 
        m_remain, frag->cells_count, frag->cells_count? ts/frag->cells_count: 0, 
        m_cells.size(), frag->interval.key_begin.to_string().c_str()
      );
    }

    if(!err && !isloaded) {
      m_queue.deactivate();
      return;
    }
    if(err)
      SWC_LOGF(LOG_ERROR, 
        "COMPACT-FRAGMENTS fragment err=%d(%s) %s", 
        err, Error::get_text(err), frag->to_string().c_str()
      );
    m_cv.notify_one();

    write(false);

  } while(!m_queue.deactivating());
  
  m_cv.notify_one();
}

void Compact::Group::write(bool last) {
  if(last && (compact->log->stopping || error))
    m_sem.wait_all();

  if(compact->log->stopping || error || m_cells.empty())
    return;
  
  if(!last &&
      m_cells.size() < compact->log->range->cfg->block_cells() 
                        * Fragments::MAX_COMPACT &&
      m_cells.size_bytes() < compact->log->range->cfg->block_size() 
                              * Fragments::MAX_COMPACT)
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
    if(last)
      m_sem.wait_until_under(5);
  } while(last && !error && !m_cells.empty());

  if(compact->log->stopping || error || last)
    m_sem.wait_all();
  else
    m_sem.wait_until_under(5);
}

void Compact::Group::finalize() {
  compact->applying();

  int err = Error::OK;
  for(auto frag : m_fragments) {
    if(compact->log->stopping || error) {
      frag->remove(err);
    } else {
      compact->log->take_ownership(err, frag);
    }
    if(err)
      error = err;
    delete frag;
  }

  if(!compact->log->stopping && !error)
    compact->log->remove(err, m_remove);

  compact->finished(this);
}


Compact::Compact(Fragments* log, const Types::KeySeq key_seq,
                 int tnum, const std::vector<Fragment::Ptr>& frags)
                : log(log),  key_seq(key_seq), ts(Time::now_ns()),
                  repetition(tnum), nfrags(frags.size()), 
                  m_workers(1), m_applying(0)  {

  uint32_t workers = Env::Resources.avail_ram()/log->range->cfg->block_size();
  if((workers /= 2) > nfrags)
    workers = nfrags;
  if(workers > 100)
    workers = Fragments::MAX_COMPACT;
  else
    workers /= Fragments::MAX_COMPACT;
  if(!workers)
    ++workers;
  m_workers = workers;

  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-START %d/%d workers=%lld fragments=%lld repetition=%d",
    log->range->cfg->cid, log->range->rid, 
    workers, nfrags, repetition
  );

  for(uint8_t w = 0; w < workers;)
    m_groups.push_back(new Group(this, ++w));

  auto it = frags.begin();
  for(uint8_t w = 0; w < workers; ++w, it += Fragments::MAX_COMPACT) {
    asio::post(*Env::IoCtx::io()->ptr(), 
      [group=m_groups[w], 
       fragments=std::vector<Fragment::Ptr>(
          it, (frags.end() - it > Fragments::MAX_COMPACT
               ? it + Fragments::MAX_COMPACT : frags.end()) )
      ] () {  group->run(fragments); }
    );
  }
}

Compact::~Compact() { }

void Compact::applying() {
  m_mutex.lock();
  bool first = !m_applying;
  ++m_applying;
  m_mutex.unlock();
  
  if(first) {
    log->range->compacting(Range::COMPACT_APPLYING);
    log->range->blocks.wait_processing(); // sync processing state
  }
}

void Compact::finished(Group* group) {
  m_mutex.lock();
  if(!--m_applying)
    log->range->compacting(Range::COMPACT_COMPACTING);
  uint8_t running = --m_workers;
  m_mutex.unlock();

  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-PROGRESS %d/%d workers-running=%d worker=%d took=%lld",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns()-group->ts) 
  );
  if(running)
    return;

  for(auto g : m_groups)
    delete g;

  auto took = Time::now_ns() - ts;
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-FINISH %d/%d fragments=%lld repetition=%d took=%lld",
    log->range->cfg->cid, log->range->rid, nfrags, repetition, took
  );

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

