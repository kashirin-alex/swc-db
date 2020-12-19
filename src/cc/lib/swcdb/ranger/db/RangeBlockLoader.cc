/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {


SWC_SHOULD_INLINE
BlockLoader::BlockLoader(Block::Ptr block)
                        : block(block),
                          preload(
                            block->blocks->range->cfg->log_fragment_preload()),
                          count_cs_blocks(0), count_fragments(0),
                          error(Error::OK),
                          m_check_log(true), m_processing(false),
                          m_logs(0) {
}

BlockLoader::~BlockLoader() { }

SWC_SHOULD_INLINE
void BlockLoader::add(const ReqScan::Ptr& req) {
  q_req.push({.req=req, .ts=Time::now_ns()});
}

SWC_SHOULD_INLINE
void BlockLoader::run() {
  block->blocks->cellstores.load_cells(this);
  load_log(false);
}

//CellStores
void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  Core::MutexSptd::scope lock(m_mutex);
  m_cs_blocks.push(blk);
}

void BlockLoader::loaded_blk() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_processing)
      return;
    m_processing = true;
  }
  Env::Rgr::post([this](){ load_cellstores_cells(); });
}

void BlockLoader::load_cellstores_cells() {
  int err;
  bool loaded;
  for(CellStore::Block::Read::Ptr blk; ; ) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_cs_blocks.empty()) {
        m_processing = false;
        break;
      }
      loaded = (blk = m_cs_blocks.front())->loaded(err = Error::OK);
      if(!err && !loaded) {
        m_processing = false;
        return;
      }
      if(err) {
        blk->processing_decrement();
        if(!error)
          error = Error::RANGE_CELLSTORES;
      }
    }
    if(loaded) {
      blk->load_cells(err, block);
      ++count_cs_blocks;
    }
    {
      Core::MutexSptd::scope lock(m_mutex);
      m_cs_blocks.pop();
    }
  }
  if(!m_check_log.running())
    load_log(false);
}

//CommitLog

void BlockLoader::load_log(bool is_final, bool is_more) {
  check_more:
  uint8_t vol = preload;
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_logs) {
      vol -= m_logs;
      if(is_final)
        is_final = false;
    }
  }
  if(vol) {
    size_t offset = m_f_selected.size(); 
    block->blocks->commitlog.load_cells(this, is_final, m_f_selected, vol); 
    if(offset < m_f_selected.size()) {
      {
        Core::MutexSptd::scope lock(m_mutex);
        m_logs += m_f_selected.size() - offset;
      }
      for(auto it=m_f_selected.begin()+offset; it<m_f_selected.end(); ++it) {
        (*it)->load([this](const CommitLog::Fragment::Ptr& frag) { 
          loaded_frag(frag); });
        (*it)->processing_decrement();
      }
    }
  }
  bool more;
  bool wait;
  {
    Core::MutexSptd::scope lock(m_mutex);
    m_check_log.stop();
    if((!is_more && m_processing) || !m_cs_blocks.empty())
      return;
    more = is_more || (m_logs && !m_fragments.empty());
    wait = m_processing || m_logs || !m_fragments.empty();
  }
  if(more) 
    return loaded_frag(nullptr);
  if(wait)
    return;
  if(is_final)
    return completion();
  if(!m_check_log.running()) {
    is_final = true;
    goto check_more;
  }
}

void BlockLoader::loaded_frag(const CommitLog::Fragment::Ptr& frag) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(frag)
      m_fragments.push(frag);
    if(m_processing || !m_cs_blocks.empty())
      return;
    m_processing = true;
  }
  Env::Rgr::post([this](){ load_log_cells(); });
}

void BlockLoader::load_log_cells() {
  bool more;
  bool loaded;
  int err;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_fragments.empty()) {
        m_processing = false;
        break;
      }
      frag = m_fragments.front();
      m_fragments.pop();
      more = m_logs == preload;
      if((loaded = frag->loaded()))
        --m_logs;
    }
    if(!loaded) {
      // temp-check, that should not be happening
      SWC_LOG_OUT(LOG_WARN,
        frag->print(SWC_LOG_OSTREAM << "Fragment not-loaded, load-again ");
      );
      frag->load([this](const CommitLog::Fragment::Ptr& frag){
        loaded_frag(frag); });
      frag->processing_decrement();
      continue;
    }
    frag->load_cells(err = Error::OK, block);
    ++count_fragments;
    if(more && !m_check_log.running())
      Env::Rgr::post([this](){ load_log(false, true); });
  }

  if(!m_check_log.running())
    load_log(true);
}

void BlockLoader::completion() {
  SWC_ASSERT(m_fragments.empty());
  SWC_ASSERT(!m_logs);
  SWC_ASSERT(m_cs_blocks.empty());

  block->loader_loaded();
}



}}
