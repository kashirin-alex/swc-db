/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/db/Files/RangeBlockLoader.h"


namespace SWC { namespace Files { namespace Range {


BlockLoader::BlockLoader(Range::Block::Ptr block) 
                        : block(block),  
                          m_processing(false), m_err(Error::OK), 
                          m_chk_cs(false), m_checking_log(false), m_frag_ts(0) {
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
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ loaded_cellstores(); });
}

void BlockLoader::loaded_cellstores() {
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
void BlockLoader::load_log(bool final) {
  int64_t last;
  {
    std::scoped_lock lock(m_mutex);
    if(m_checking_log || m_fragments.size() == MAX_FRAGMENTS)
      return;
    m_checking_log = true;
    last = m_frag_ts;
  }

  block->blocks->commitlog.load_cells(this, final, last);
  
  bool no_more;
  std::vector<CommitLog::Fragment::Ptr> for_loading;
  {
    std::scoped_lock lock(m_mutex);
    no_more = final && m_fragments.empty() && m_chk_cs;
    for(auto frag : m_fragments)
      if(frag->ts > last)
        for_loading.push_back(frag);
    /*
    std::cout << " fragments=" << m_fragments.size() 
              << " for_loading=" << for_loading.size() 
              << " no_more=" << no_more << " last=" << last << "\n";
    */
    m_checking_log = false;
  }
  
  for(auto frag : for_loading)
    frag->load([this](){ loaded_frag(); });

  if(no_more)
    completion();
}

bool BlockLoader::add(CommitLog::Fragment::Ptr frag) {
  std::scoped_lock lock(m_mutex);
  m_fragments.push_back(frag);
  m_frag_ts = frag->ts;
  return m_fragments.size() == MAX_FRAGMENTS;
}

void BlockLoader::loaded_frag() {
  { 
    std::scoped_lock lock(m_mutex);
    if(m_processing || !m_chk_cs || !m_cs_blocks.empty()) 
      return;
    m_processing = true;
  }
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ loaded_log(); });
}

void BlockLoader::loaded_log() {
  int err;
  bool loaded;
  bool load_more;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {          
      std::scoped_lock lock(m_mutex);        
      if(m_fragments.empty()) {
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
      load_more = m_fragments.size() == 3;
      m_fragments.erase(m_fragments.begin());
    }
    
    if(load_more)
      asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_log(false) });
  }

  load_log(true);
}

void BlockLoader::completion() {
  block->blocks->commitlog.load_cells(this);
  block->loaded(m_err);
  delete this;
}



}}}
