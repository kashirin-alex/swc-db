/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {


BlockLoader::BlockLoader(Block::Ptr block) 
                        : block(block),  
                          m_processing(false), m_err(Error::OK), 
                          m_chk_cs(false), m_checking_log(true), m_frag_ts(0) {
}

BlockLoader::~BlockLoader() { }

void BlockLoader::run() {
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_cellstores(); });
  load_log(false);
}

//CellStores
void BlockLoader::load_cellstores() {
  block->blocks->cellstores.load_cells(this);

  {
    std::scoped_lock lock(m_mutex);
    m_chk_cs = true;
  }
  
  loaded_frag();
}

void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  std::scoped_lock lock(m_mutex);
  m_cs_blocks.push_back(blk);
  /*
  if(m_cs_blocks.size() > 4) {
    SWC_LOG_OUT(LOG_DEBUG);
    std::cout << " BlockLoader c=" << m_cs_blocks.size() << " CS\n";
    std::cout << " block :" << block->to_string() << "\n";
    int n = 0;
    for(auto b  : m_cs_blocks)
      std::cout << " csblk "<< ++n << "=" << b->to_string() << "\n";
    std::cout << SWC_LOG_OUT_END;
  }
  */
}

void BlockLoader::loaded_blk() {
  { 
    std::scoped_lock lock(m_mutex);
    if(m_processing) 
      return;
    m_processing = true;
  }
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_cellstores_cells(); });
}

void BlockLoader::load_cellstores_cells() {
  int err;
  bool loaded;
  for(CellStore::Block::Read::Ptr blk; ; ) {
    {
      std::scoped_lock lock(m_mutex);
      if(m_cs_blocks.empty()) {
        m_processing = false;
        break;
      }
      blk = m_cs_blocks.front();
    }

    if(loaded = blk->loaded(err))
      blk->load_cells(err, block);
      
    std::scoped_lock lock(m_mutex);
    if(!err && !loaded) {
      m_processing = false;
      return;
    }
    
    if(err) {
      blk->processing_decrement();
      if(!m_err)
        m_err = Error::RANGE_CELLSTORES;
    }
    m_cs_blocks.erase(m_cs_blocks.begin());
  }

  loaded_frag();
}

//CommitLog
bool BlockLoader::check_log() {
  std::scoped_lock lock(m_mutex);
  if(m_checking_log)
    return false;
  m_checking_log = true;
  return true;
}

void BlockLoader::load_log(bool is_final) {
  std::vector<CommitLog::Fragment::Ptr> need_load;
  block->blocks->commitlog.load_cells(this, is_final, m_frag_ts, need_load);
  if(!need_load.empty()) {
    m_frag_ts = need_load.back()->ts;
    {
      std::scoped_lock lock(m_mutex);
      m_fragments.insert(
        m_fragments.end(), need_load.begin(), need_load.end());
    }
    for(auto frag : need_load)
      frag->load([this](){ loaded_frag(); });
  }

  bool no_more;
  {
    std::scoped_lock lock(m_mutex);
    no_more = !m_processing && m_fragments.empty() && m_chk_cs;
    m_checking_log = false;
  }
  if(no_more) {
    if(is_final)
      completion();
    else
      loaded_frag();
  }
}

void BlockLoader::loaded_frag() {
  {
    std::scoped_lock lock(m_mutex);
    if(m_processing || !m_chk_cs || !m_cs_blocks.empty()) 
      return;
    m_processing = true;
  }
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_log_cells(); });
}

void BlockLoader::load_log_cells() {
  int err;
  bool loaded;
  size_t sz;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {          
      std::scoped_lock lock(m_mutex);
      if(!(sz = m_fragments.size())) {
        m_processing = false;
        break;
      }
      frag = m_fragments.front();
    }

    if(loaded = frag->loaded(err))
      frag->load_cells(err, block);
    {
      std::scoped_lock lock(m_mutex);
      if(!err && !loaded) {
        m_processing = false;
        return;
      }

      if(err) {
        frag->processing_decrement();
        if(!m_err)
          m_err = Error::RANGE_COMMITLOG;
      }
      m_fragments.erase(m_fragments.begin());
    }

    if(sz == MAX_FRAGMENTS && check_log())
      asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_log(false); });
  }

  if(check_log())
    load_log(true);
}

void BlockLoader::completion() {
  block->blocks->commitlog.load_cells(this);
  block->loaded(m_err);
  
  assert(m_fragments.empty());
  assert(m_cs_blocks.empty());
  
  delete this;
}



}}
