/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {


SWC_SHOULD_INLINE
BlockLoader::BlockLoader(Block::Ptr block)
                        : block(block),
                          count_cs_blocks(0), count_fragments(0),
                          error(Error::OK),
                          m_sem_cs(2, 1),
                          m_sem_log(CommitLog::Fragments::MAX_PRELOAD),
                          m_cs_blocks_running(false),
                          m_fragments_running(false) {
}

BlockLoader::~BlockLoader() { }

SWC_SHOULD_INLINE
void BlockLoader::run(bool preloading) {
  if(preloading) {
    block->blocks->cellstores.load_cells(this);
    m_sem_cs.release();
  } else {
    Env::Rgr::post([this]() {
      block->blocks->cellstores.load_cells(this);
      m_sem_cs.release();
      load_fragments();
    });
  }
  Env::Rgr::post([this](){ load_log(); });
}

void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  m_sem_cs.acquire();
  Core::MutexSptd::scope lock(m_mutex);
  m_cs_blocks.push(blk);
}

void BlockLoader::load_log() {
  bool is_final = false;
  std::vector<CommitLog::Fragment::Ptr> selected;
  for(size_t offset=0; ;) {
    uint8_t sz = m_sem_log.wait_available();
    block->blocks->commitlog.load_cells(this, is_final, selected, sz);
    if(is_final)
      break;
    if(offset < selected.size()) {
      for(auto it=selected.begin()+offset; it < selected.end(); ++it) {
        m_sem_log.acquire();
        {
          Core::MutexSptd::scope lock(m_mutex);
          m_fragments.push(*it);
        }
        (*it)->load([this](CommitLog::Fragment::Ptr){ load_fragments(); });
        (*it)->processing_decrement();
      }
      offset = selected.size();
    } else {
      is_final = true;
      m_sem_cs.wait_all();
      m_sem_log.wait_all();
    }
  }
  block->loaded(this);
}

void BlockLoader::loaded_blk() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_cs_blocks_running)
      return;
    m_cs_blocks_running = true;
  }
  Env::Rgr::post([this](){ load_cs_blocks(); });
}

void BlockLoader::load_cs_blocks() {
  int err;
  for(CellStore::Block::Read::Ptr blk; ; ) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_cs_blocks.empty()) {
        m_cs_blocks_running = false;
        break;
      }
      blk = m_cs_blocks.front();
      if(blk->loaded(err) || err) {
        m_cs_blocks.pop();
      } else {
        m_cs_blocks_running = false;
        return;
      }
    }
    if(err) {
      blk->processing_decrement();
      if(!error)
        error = Error::RANGE_CELLSTORES;
    } else {
      blk->load_cells(err, block);
    }
    m_sem_cs.release();
    ++count_cs_blocks;
  }
  load_fragments();
}

void BlockLoader::load_fragments() {
  if(m_sem_cs.has_pending())
    return;
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_fragments_running)
      return;
    m_fragments_running = true;
  }
  Env::Rgr::post([this](){ _load_fragments(); });
}

void BlockLoader::_load_fragments() {
  int err;
  for(CommitLog::Fragment::Ptr frag; ; ) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_fragments.empty()) {
        m_fragments_running = false;
        return;
      }
      frag = m_fragments.front();
      if(frag->loaded(err)) {
        m_fragments.pop();
        goto load_cells;
      }
      m_fragments_running = false;
    }

    if(err) {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(
          SWC_LOG_OSTREAM << "BlockLoader fragment retrying to ", err);
        frag->print(SWC_LOG_OSTREAM << ' ');
      );
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      frag->load([this](CommitLog::Fragment::Ptr) { load_fragments(); } );
      frag->processing_decrement();
    }
    return;

    load_cells:
      frag->load_cells(err, block);
      m_sem_log.release();
      ++count_fragments;
  }
}



}}
