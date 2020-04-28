/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/ranger/db/CommitLogCompact.h"

namespace SWC { namespace Ranger { namespace CommitLog {

Compact::Group::Group(Compact* compact, uint8_t worker) 
                      : ts(Time::now_ns()), worker(worker), error(Error::OK), 
                        compact(compact), read_idx(0), 
                        m_cells(
                          compact->key_seq,
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
  for(;read_idx < read_frags.size(); ++read_idx) {
    if(m_queue.size() == Fragments::MAX_PRELOAD)
      return;
    if(error || compact->log->stopping) {
      read_idx = read_frags.size();
      break;
    }
    auto& frag = read_frags[read_idx];
    m_queue.push(frag);
    frag->load([this]() { loaded(); loaded(); });
  }
  if(read_idx == read_frags.size()) {
    m_queue.push(nullptr);
    if(m_queue.activating())
      load();
  }
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
    if(!(frag = m_queue.front())) {
      finalize();
      return;
    }
    if(m_queue.size() < Fragments::MAX_PRELOAD) 
      load_more();

    ts = Time::now_ns();
    err = Error::OK;
    if(isloaded = frag->loaded(err)) {
      if(!compact->log->stopping && !error) {
        frag->load_cells(err, m_cells); 
        m_remove.push_back(frag);
      } else {
        frag->processing_decrement();
        frag->release();
      }
    }
    if(!err && !isloaded) {
      m_queue.deactivate();
      return;
    }
    if(err)
      SWC_LOGF(LOG_ERROR, 
        "COMPACT-FRAGMENTS-ERROR %d/%d err=%d(%s) %s", 
        compact->log->range->cfg->cid, compact->log->range->rid,
        err, Error::get_text(err), frag->to_string().c_str()
      );


    if(!compact->log->stopping && !error) {
      ts = Time::now_ns() - ts;
      SWC_LOGF(LOG_INFO, 
        "COMPACT-FRAGMENTS-PROGRESS %d/%d w=%d(%lld/%lld) %lld(%lldns/cell)"
        " sz=%lld begin-%s", 
        compact->log->range->cfg->cid, compact->log->range->rid, 
        worker, m_remove.size(), read_frags.size(), 
        frag->cells_count, frag->cells_count? ts/frag->cells_count: 0, 
        m_cells.size(), frag->interval.key_begin.to_string().c_str()
      );
    }

  } while(!m_queue.deactivating());
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
}

void Compact::Group::finalize() {
  write();

  compact->applying();

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

  compact->finished(this);
}


Compact::Compact(Fragments* log, const Types::KeySeq key_seq,
                 int tnum, const std::vector<Fragment::Ptr>& frags)
                : log(log),  key_seq(key_seq), ts(Time::now_ns()),
                  repetition(tnum), nfrags(frags.size()), 
                  m_applying(0) {
  uint32_t workers = Env::Resources.avail_ram()/log->range->cfg->block_size();
  if(workers > nfrags)
    workers = nfrags;
  if(workers > 100)
    workers = Fragments::MAX_COMPACT;
  else
    workers /= Fragments::MAX_COMPACT;
  if(!workers)
    ++workers;

  for(auto it = frags.begin(); it < frags.end(); ++it) {
    if(m_groups.size() == 0 || 
       m_groups.back()->read_frags.size() == Fragments::MAX_COMPACT) {
      if(m_groups.size() == workers)
        break;
      m_groups.push_back(new Group(this, m_groups.size()+1));
    }
    m_groups.back()->read_frags.push_back(*it);
  }
  m_workers = m_groups.size();
  
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-START %d/%d workers=%lld fragments=%lld repetition=%d",
    log->range->cfg->cid, log->range->rid, 
    workers, nfrags, repetition
  );

  for(auto g : m_groups)
    g->run();
}

Compact::~Compact() { }

void Compact::applying() {
  std::scoped_lock lock(m_mutex);
  if(!m_applying) {
    log->range->compacting(Range::COMPACT_APPLYING);
    log->range->blocks.wait_processing(); // sync processing state
  }
  ++m_applying;
}

void Compact::finished(Group* group) {
  uint8_t running;
  {
    std::scoped_lock lock(m_mutex);
    if(!--m_applying)
      log->range->compacting(Range::COMPACT_COMPACTING);
    running = --m_workers;
  }
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-PROGRESS %d/%d running=%d worker=%d-%lldns",
    log->range->cfg->cid, log->range->rid, running, 
    group->worker, (Time::now_ns()-group->ts) 
  );
  if(running)
    return;

  for(auto g : m_groups)
    delete g;

  auto took = Time::now_ns() - ts;
  SWC_LOGF(LOG_INFO, 
    "COMPACT-FRAGMENTS-FINISH %d/%d fragments=%lld repetition=%d %lldns",
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

