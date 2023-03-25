/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {


SWC_CAN_INLINE
BlockLoader::BlockLoader(Block::Ptr a_block)
                        : block(a_block),
                          count_cs_blocks(0), count_fragments(0),
                          q_req(),
                          error(Error::OK),
                          preload(
                            block->blocks->range->cfg->log_fragment_preload()),
                          m_check_log(true), m_processing(false),
                          m_logs(0),
                          m_mutex(),
                          m_cs_blocks(), m_f_selected(), m_fragments() {
}

SWC_CAN_INLINE
void BlockLoader::add(const ReqScan::Ptr& req) {
  q_req.push({.req=req, .ts=Time::now_ns()});
}

SWC_CAN_INLINE
void BlockLoader::run() {
  block->blocks->cellstores.load_cells(this);
  load_log(false);
}

//CellStores
SWC_CAN_INLINE
void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  Core::MutexSptd::scope lock(m_mutex);
  m_cs_blocks.push(blk);
}

SWC_CAN_INLINE
void BlockLoader::loaded_blk() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_processing)
      return;
    m_processing = true;
  }
  struct Task {
    BlockLoader* ptr;
    SWC_CAN_INLINE
    Task(BlockLoader* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()() { ptr->load_cellstores_cells(); }
  };
  Env::Rgr::post(Task(this));
}

SWC_CAN_INLINE
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
        err = Error::OK;
        error.compare_exchange_weak(err, Error::RANGE_CELLSTORES);
      }
    }
    if(loaded) {
      blk->load_cells(err, block);
      count_cs_blocks.fetch_add(1);
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
      for(auto it=m_f_selected.cbegin() + offset;
          it != m_f_selected.cend(); ++it) {
        (*it)->load(this);
      }
    }
  }
  bool more;
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(((!is_more && m_processing) || !m_cs_blocks.empty()) ||
        (!(more = is_more || (m_logs && !m_fragments.empty())) &&
         (m_processing || m_logs || !m_fragments.empty())))  {
      m_check_log.stop();
      return;
    }
  }
  if(more) {
    m_check_log.stop();
    return loaded(nullptr);
  }
  if(is_final)
    return completion();
  is_final = true;
  goto check_more;
}

void BlockLoader::loaded(CommitLog::Fragment::Ptr&& frag) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(frag)
      m_fragments.push(std::move(frag));
    if(m_processing || !m_cs_blocks.empty())
      return;
    m_processing = true;
  }
  struct Task {
    BlockLoader* ptr;
    SWC_CAN_INLINE
    Task(BlockLoader* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()() { ptr->load_log_cells(); }
  };
  Env::Rgr::post(Task(this));
}

SWC_CAN_INLINE
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
      frag = std::move(m_fragments.front());
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
      frag->load(this);
      continue;
    }
    frag->load_cells(err = Error::OK, block);
    count_fragments.fetch_add(1);
    if(more && !m_check_log.running()) {
      struct Task {
        BlockLoader* ptr;
        SWC_CAN_INLINE
        Task(BlockLoader* a_ptr) noexcept : ptr(a_ptr) { }
        void operator()() { ptr->load_log(false, true); }
      };
      Env::Rgr::post(Task(this));
    }
  }

  if(!m_check_log.running())
    load_log(true);
}

SWC_CAN_INLINE
void BlockLoader::completion() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(!m_cs_blocks.empty()) {
      SWC_LOG_OUT(LOG_ERROR,
        SWC_LOG_OSTREAM << "Not Loaded CellStores: " << m_cs_blocks.size();
        for(; !m_cs_blocks.empty(); m_cs_blocks.pop()) {
          m_cs_blocks.front()->print(SWC_LOG_OSTREAM << "\n\t");
          m_cs_blocks.front()->processing_decrement();
        }
      );
      error.store(Error::RANGE_CELLSTORES);
    }
    if(m_logs || !m_fragments.empty()) {
      SWC_LOG_OUT(LOG_ERROR,
        SWC_LOG_OSTREAM << "Not Loaded Log Fragments: " << m_fragments.size();
        for(; !m_fragments.empty(); m_fragments.pop()) {
          m_fragments.front()->print(SWC_LOG_OSTREAM << "\n\t");
          m_fragments.front()->processing_decrement();
        }
      );
      if(!error)
        error.store(Error::RANGE_COMMITLOG);
    }
  }

  block->loader_loaded();
}



}}
