/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeBlockLoader.h"


namespace SWC { namespace Ranger {

class BlockLoader::LoadQueue {
  public:
  LoadQueue() noexcept { }
  LoadQueue(LoadQueue&&) = delete;
  LoadQueue(const LoadQueue&) = delete;
  LoadQueue& operator=(LoadQueue&&) = delete;
  LoadQueue& operator=(const LoadQueue&) = delete;
  virtual ~LoadQueue() noexcept { }
  virtual void load_cells(BlockLoader* loader) = 0;
};

template<typename ItemT, int ErrorT>
class LoadQueueItem final : public BlockLoader::LoadQueue {
  public:

  LoadQueueItem(ItemT&& a_ptr, size_t& a_counter) noexcept
                : ptr(std::move(a_ptr)), counter(a_counter) {
  }

  LoadQueueItem(LoadQueueItem&&) = delete;
  LoadQueueItem(const LoadQueueItem&) = delete;
  LoadQueueItem& operator=(LoadQueueItem&&) = delete;
  LoadQueueItem& operator=(const LoadQueueItem&) = delete;

  virtual ~LoadQueueItem() noexcept { }

  virtual void load_cells(BlockLoader* loader) override {
    int err = Error::OK;
    bool loaded = ptr->loaded(err);

    if(loaded)
      ptr->load_cells(err, loader->block);

    if(err) {
      if(!loader->error)
        loader->error = ErrorT;
      SWC_LOG_OUT(
        LOG_WARN,
        ptr->print(SWC_LOG_OSTREAM << "BlockLoader error(" << err << ')');
      );
      if(!loaded)
        ptr->processing_decrement();
    }
    ++counter;
  }

  private:
  ItemT   ptr;
  size_t& counter;
};



SWC_CAN_INLINE
BlockLoader::BlockLoader(Block::Ptr a_block)
  : block(a_block),
    count_cs_blocks(0), count_fragments(0),
    q_req(),
    error(Error::OK),
    preload(block->blocks->range->cfg->log_fragment_preload()),
    m_ack(false), m_logs(0), m_cs_blks(1),
    m_mutex(), m_cv(), m_queue(), m_f_selected(),
    m_cs_selected(), m_cs_ready() {
}

SWC_CAN_INLINE
void BlockLoader::add(const ReqScan::Ptr& req) {
  q_req.push({.req=req, .ts=Time::now_ns()});
}

SWC_CAN_INLINE
void BlockLoader::run() {
  struct Task {
    BlockLoader* ptr;
    SWC_CAN_INLINE
    Task(BlockLoader* a_ptr) noexcept : ptr(a_ptr) { }
    Task(Task&& other)  noexcept : ptr(other.ptr) { }
    Task(const Task&) = delete;
    Task& operator=(Task&&) = delete;
    Task& operator=(const Task&) = delete;
    ~Task() noexcept { }
    void operator()() { ptr->load_cells(); }
  };
  Env::Rgr::block_loader_post(Task(this));

  block->blocks->cellstores.load_cells(this);

  Core::ScopedLock lock(m_mutex);
  --m_cs_blks;
  m_ack = true;
  m_cv.notify_all();
}

//CellStores
SWC_CAN_INLINE
void BlockLoader::add(CellStore::Block::Read::Ptr blk) {
  Core::ScopedLock lock(m_mutex);
  ++m_cs_blks;
  m_cs_selected.push(blk);
}

SWC_CAN_INLINE
void BlockLoader::loaded(CellStore::Block::Read::Ptr&& blk) {
  Core::ScopedLock lock(m_mutex);
  m_cs_ready.push_back(blk);
  for(auto it = m_cs_ready.begin();
      it < m_cs_ready.end() && m_cs_selected.front() == *it; ) {
    m_queue.push(
      new LoadQueueItem<
        CellStore::Block::Read::Ptr,
        Error::RANGE_CELLSTORES
      >(std::move(*it), count_cs_blocks)
    );
    m_cs_ready.erase(it);
    m_cs_selected.pop();
    --m_cs_blks;
  }
  m_ack = true;
  m_cv.notify_all();
}

void BlockLoader::loaded(CommitLog::Fragment::Ptr&& frag) {
  LoadQueue* ptr(
    new LoadQueueItem<
      CommitLog::Fragment::Ptr,
      Error::RANGE_COMMITLOG
    >(std::move(frag), count_fragments)
  );
  Core::ScopedLock lock(m_mutex);
  m_queue.push(ptr);
  --m_logs;
  m_ack = true;
  m_cv.notify_all();
}

SWC_CAN_INLINE
void BlockLoader::load_cells() {
  for(LoadQueue* ptr; ;) {
    size_t sz;
    {
      Core::UniqueLock lock_wait(m_mutex);
      if(m_ack)
        m_cv.wait(lock_wait, [this] { return m_ack; });
      m_ack = false;
      if((ptr = m_queue.empty() ? nullptr : m_queue.front()))
        m_queue.pop();
      sz = m_cs_blks ? preload : (m_queue.size() + m_logs);
    }
    bool is_final = false;
    if(sz < preload) {
      uint8_t vol = preload - sz;
      if((is_final = vol == preload) && ptr) {
        ptr->load_cells(this);
        delete ptr;
        ptr = nullptr;
      }
      size_t offset = m_f_selected.size();
      block->blocks->commitlog.load_cells(this, is_final, m_f_selected, vol);
      if((sz = m_f_selected.size() - offset)) {
        {
          Core::ScopedLock lock(m_mutex);
          m_logs += sz;
        }
        for(auto it=m_f_selected.cbegin() + offset;
            it != m_f_selected.cend(); ++it) {
          (*it)->load(this);
        }
      } else if(is_final) {
        SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "Loaded "););
        block->loader_loaded();
        break;
      }
    }
    if(ptr) {
      ptr->load_cells(this);
      delete ptr;
      ptr = nullptr;
    }
  }
}


}}
