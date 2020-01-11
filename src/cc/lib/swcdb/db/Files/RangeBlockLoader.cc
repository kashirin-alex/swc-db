/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/db/Files/RangeBlockLoader.h"


namespace SWC { namespace Files { namespace Range {


BlockLoader::BlockLoader(Range::Block::Ptr block) 
                        : block(block),  
                          m_processing(false), m_err(Error::OK), 
                          m_chk_cs(false), m_chk_log(false) {
}

BlockLoader::~BlockLoader() { }

void BlockLoader::run() {
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_cellstores(); });
  load_log();//asio::post(*Env::IoCtx::io()->ptr(), [this](){ load_log(); });
}

void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  std::scoped_lock<std::mutex> lock(m_mutex);
  m_cs_blocks.push_back(blk);
}

void BlockLoader::add(CommitLog::Fragment::Ptr frag) {
  std::scoped_lock<std::mutex> lock(m_mutex);
  m_fragments.push_back(frag);
}

void BlockLoader::loaded_blk() {
  { 
    std::scoped_lock<std::mutex> lock(m_mutex);
    //SWC_LOGF(LOG_DEBUG, " BlockLoader, CellStore::Block count=%d", 
    //         m_cs_blocks.size());
    if(m_processing) 
      return;
    m_processing = true;
  }
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ loaded_cellstores(); });
}

void BlockLoader::loaded_frag() {
  { 
    std::scoped_lock<std::mutex> lock(m_mutex);
    //SWC_LOGF(LOG_DEBUG, " BlockLoader, Fragment count=%d", 
    //         m_fragments.size());
    if(m_processing || !m_chk_cs || !m_cs_blocks.empty()) 
      return;
    m_processing = true;
  }
  asio::post(*Env::IoCtx::io()->ptr(), [this](){ loaded_log(); });
}

void BlockLoader::load_cellstores() {
  block->blocks->cellstores.load_cells(this);
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_chk_cs = true;
  }
  completion();
}

void BlockLoader::load_log() {
  block->blocks->commitlog.load_cells(this, false);
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_chk_log = true;
  }
  completion();
}

void BlockLoader::loaded_cellstores() {
  int err;
  bool loaded;
  for(CellStore::Block::Read::Ptr blk; ; ) {
    {
      std::scoped_lock<std::mutex> lock(m_mutex);
      if(m_cs_blocks.empty()) {
        m_processing = false;
        break;
      }
      blk = m_cs_blocks.front();
    }

    if(loaded = blk->loaded(err))
      blk->load_cells(err, block);
      
    std::scoped_lock<std::mutex> lock(m_mutex);
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

  completion();
}

void BlockLoader::loaded_log() {
  int err;
  bool loaded;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {          
      std::scoped_lock<std::mutex> lock(m_mutex);        
      if(m_fragments.empty()) {
        m_processing = false;
        break;
      }
      frag = m_fragments.front();
    }

    if(loaded = frag->loaded(err))
      frag->load_cells(err, block);
        
    std::scoped_lock<std::mutex> lock(m_mutex);
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
  
  completion();
}

void BlockLoader::completion() { 
  bool more_log;   
  {
    std::scoped_lock<std::mutex> lock(m_mutex);   
    if(m_processing || !m_chk_cs || !m_chk_log || !m_cs_blocks.empty()) 
      return;
    more_log = !m_fragments.empty();
    m_processing = more_log;
  }

  if(more_log) {
    asio::post(*Env::IoCtx::io()->ptr(), [this](){ loaded_log(); } );

  } else if(block->blocks->commitlog.load_cells(this, true, ts)) {
    block->loaded(m_err);
    delete this;
  }
}



}}}
