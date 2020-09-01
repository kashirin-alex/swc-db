/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {


SWC_SHOULD_INLINE
BlockLoader::BlockLoader(Block::Ptr block) 
                        : block(block), m_err(Error::OK), 
                          m_processing(false),
                          m_checking_log(true), m_logs(0) {
}

BlockLoader::~BlockLoader() { }

SWC_SHOULD_INLINE
void BlockLoader::run() {
  block->blocks->cellstores.load_cells(this);
  load_log(false);
}

//CellStores
void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  Mutex::scope lock(m_mutex);
  m_cs_blocks.push(blk);
}

void BlockLoader::loaded_blk() {
  { 
    Mutex::scope lock(m_mutex);
    if(m_processing) 
      return;
    m_processing = true;
  }
  Env::IoCtx::post([this](){ load_cellstores_cells(); });
}

void BlockLoader::load_cellstores_cells() {
  int err;
  bool loaded;
  for(CellStore::Block::Read::Ptr blk; ; ) {
    {
      Mutex::scope lock(m_mutex);
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
        if(!m_err)
          m_err = Error::RANGE_CELLSTORES;
      }
    }
    if(loaded)
      blk->load_cells(err, block);
    {
      Mutex::scope lock(m_mutex);
      m_cs_blocks.pop();
    }
  }
  if(check_log())
    load_log(false);
}

//CommitLog
bool BlockLoader::check_log() {
  Mutex::scope lock(m_mutex);
  return m_checking_log ? false : (m_checking_log = true);
}

void BlockLoader::load_log(bool is_final, bool is_more) {
  uint8_t vol = CommitLog::Fragments::MAX_PRELOAD;
  {
    Mutex::scope lock(m_mutex);
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
        Mutex::scope lock(m_mutex);
        m_logs += m_f_selected.size() - offset;
      }
      for(auto it=m_f_selected.begin()+offset; it < m_f_selected.end(); ++it) {
        (*it)->load([this, frag=*it](){ loaded_frag(frag); });
        (*it)->processing_decrement();
      }
    }
  }
  bool more;
  bool wait;
  {
    Mutex::scope lock(m_mutex);
    m_checking_log = false;
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
  if(check_log())
    return load_log(true);
}

void BlockLoader::loaded_frag(CommitLog::Fragment::Ptr frag) {
  {
    Mutex::scope lock(m_mutex);
    if(frag)
      m_fragments.push(frag);
    if(m_processing || !m_cs_blocks.empty())
      return;
    m_processing = true;
  }
  Env::IoCtx::post([this](){ load_log_cells(); });
}

void BlockLoader::load_log_cells() { 
  bool more;
  bool loaded;
  int err;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {
      Mutex::scope lock(m_mutex);
      if(m_fragments.empty()) {
        m_processing = false;
        break;
      }
      frag = m_fragments.front();
      m_fragments.pop();
      more = m_logs == CommitLog::Fragments::MAX_PRELOAD;
      if((loaded = frag->loaded()))
        --m_logs;
    }
    if(!loaded) {
      // temp-check, that should not be happening
      SWC_LOGF(LOG_WARN, "Fragment not-loaded, load-again %s", 
                frag->to_string().c_str());
      frag->load([this, frag](){ loaded_frag(frag); });
      frag->processing_decrement();
      continue;
    }
    frag->load_cells(err = Error::OK, block);
    if(more && check_log())
      Env::IoCtx::post([this](){ load_log(false, true); });
  }

  if(check_log())
    load_log(true);
}

void BlockLoader::completion() {
  SWC_ASSERT(m_fragments.empty());
  SWC_ASSERT(!m_logs);
  SWC_ASSERT(m_cs_blocks.empty());

  block->loaded(m_err, this);
}



}}
