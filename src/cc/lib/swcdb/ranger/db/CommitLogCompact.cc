/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {


Compact::Compact(Fragments* log, const Types::KeySeq key_seq) 
                : stop(false), error(Error::OK), 
                  log(log), m_cells(key_seq), 
                  m_remain(0), m_sem(5) {
}

Compact::~Compact() { }

void Compact::run(int tnum, const std::vector<Fragment::Ptr>& frags) {
  uint64_t ts = Time::now_ns();

  m_cells.reset(
    log->range->cfg->cell_versions(), 
    log->range->cfg->cell_ttl(), 
    log->range->cfg->column_type()
  );
  m_remain = frags.size();

  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-START %d/%d fragments=%lld repetition=%d",
    log->range->cfg->cid, log->range->rid, m_remain, tnum
    );

  Fragment::Ptr frag;
  auto max = Fragments::MAX_COMPACT;
  for(auto it = frags.begin(); max && it<frags.end(); ++it, --max) {
    if(m_queue.size() == Fragments::MAX_PRELOAD) {
      std::unique_lock lock_wait(m_mutex);
      m_cv.wait(lock_wait, [this]() { 
        return m_queue.size() < Fragments::MAX_PRELOAD; 
      });
    }
    if(error || stop)
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

  ts = Time::now_ns() - ts;
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-FINISH %d/%d fragments=%lld repetition=%d"
    " avg=%lldns took=%lld",
    log->range->cfg->cid, log->range->rid, frags.size(), tnum,
    ts / frags.size(), ts
    );
  
}

void Compact::loaded() {
  if(m_queue.activating())
    asio::post(*Env::IoCtx::io()->ptr(), [this](){ load(); });
}

void Compact::load() {
  bool loaded;
  int err = Error::OK;
  Fragment::Ptr frag;
  uint64_t ts;
  do {
    err = Error::OK;
    if(stop || error) {
      do {
        (frag = m_queue.front())->processing_decrement();
        frag->release();
      } while(!m_queue.deactivating());
      break;
    }

    if(loaded = (frag = m_queue.front())->loaded(err)) {
      ts = Time::now_ns();
      frag->load_cells(err, m_cells); 
      m_remove.push_back(frag);
      --m_remain;

      ts = Time::now_ns() - ts;
      SWC_LOGF(LOG_INFO, 
        "COMPACT-FRAGMENTS-PROGRESS %d/%d remain=%lld "
        "cells=%lld avg=%lldns took=%lld compacting=%lld begin-%s end-%s", 
        log->range->cfg->cid, log->range->rid, m_remain,
        frag->cells_count, frag->cells_count? ts/frag->cells_count: 0, 
        ts, m_cells.size(), 
        frag->interval.key_begin.to_string().c_str(),
        frag->interval.key_end.to_string().c_str()
      );
    }

    if(!err && !loaded) {
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

void Compact::write(bool last) {
  if(last && (stop || error))
    m_sem.wait_all();
  if(stop || error || m_cells.empty())
    return;
  
  auto bytes = log->range->cfg->block_size();
  auto cells = log->range->cfg->block_cells();
  if(!last) {
    auto ratio = log->range->cfg->log_rollout_ratio();
    if(ratio < Fragments::MAX_COMPACT)
      ratio = Fragments::MAX_COMPACT;
    bool ok = (
        m_cells.size() >= cells * ratio ||
        m_cells.size_bytes() >= bytes * ratio 
      ) && (
        Env::Resources.need_ram(bytes * ratio) || 
        m_cells.size() >= cells * ratio * 2 // huge-cells => (log-cells-blocks)
      );
    if(!ok)
      return;
  }

  int err = Error::OK;
  do {
    DynamicBuffer cells;
    auto frag = Fragment::make(
      get_filepath(Time::now_ns()), 
      m_cells.key_seq, 
      Fragment::State::WRITING
    );
    m_fragments.push_back(frag);

    m_cells.write_and_free(
      cells, 
      frag->cells_count, frag->interval, 
      log->range->cfg->block_size(), log->range->cfg->block_cells()
    );

    frag->write(
      err, 
      log->range->cfg->file_replication(), 
      log->range->cfg->block_enc(), 
      cells, 
      log->range->cfg->cell_versions(),
      &m_sem
    );
    if(err)
      error = err;
    if(last)
      m_sem.wait_until_under(5);
  } while(last && !error && !m_cells.empty());

  if(stop || error || last)
    m_sem.wait_all();
  else
    m_sem.wait_until_under(5);
}

void Compact::finalize() {
  log->range->compacting(Range::COMPACT_APPLYING);
  log->range->blocks.wait_processing(); // sync processing state

  int err = Error::OK;
  for(auto frag : m_fragments) {
    if(stop || error) {
      frag->remove(err);
    } else {
      log->take_ownership(err, frag);
    }
    if(err)
      error = err;
    delete frag;
  }

  if(!stop && !error)
    log->remove(err, m_remove);
  m_remove.clear();
  m_fragments.clear();

  log->range->compacting(Range::COMPACT_NONE);
}

const std::string Compact::get_filepath(const int64_t frag) const {
  std::string s(log->range->get_path(Range::LOG_TMP_DIR));
  s.append("/");
  s.append(std::to_string(frag));
  s.append(".frag");
  return s;
}

}}} // namespace SWC::Ranger::CommitLog

