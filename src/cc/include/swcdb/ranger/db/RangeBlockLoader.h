/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_BlockLoader_h
#define swcdb_ranger_db_BlockLoader_h


#include "swcdb/ranger/db/RangeBlock.h"
#include "swcdb/ranger/db/CellStoreReaders.h"
#include "swcdb/ranger/db/CommitLog.h"


namespace SWC { namespace Ranger {


class BlockLoader final : private CommitLog::Fragment::LoadCallback {
  public:

  Block::Ptr    block;
  size_t        count_cs_blocks;
  size_t        count_fragments;

  struct ReqQueue {
    ReqScan::Ptr  req;
    const int64_t ts;
    SWC_CAN_INLINE
    ReqQueue(const ReqScan::Ptr& a_req, int64_t a_ts) noexcept
            : req(a_req), ts(a_ts) { }
    SWC_CAN_INLINE
    ReqQueue(ReqQueue&& other) noexcept
            : req(std::move(other.req)), ts(other.ts) { }
    ReqQueue(const ReqQueue&) = delete;
    ReqQueue& operator=(ReqQueue&&) = delete;
    ReqQueue& operator=(const ReqQueue&) = delete;
    ~ReqQueue() noexcept { }
  };
  std::queue<ReqQueue> q_req; // synced by Block mutex
  int           error;
  const uint8_t preload;


  explicit BlockLoader(Block::Ptr block);

  BlockLoader(BlockLoader&&) = delete;
  BlockLoader(const BlockLoader&) = delete;
  BlockLoader& operator=(BlockLoader&&) = delete;
  BlockLoader& operator=(const BlockLoader&) = delete;

  ~BlockLoader() noexcept { }

  void add(const ReqScan::Ptr& req);

  void run();

  void add(CellStore::Block::Read::Ptr blk);

  void loaded(CellStore::Block::Read::Ptr&& blk);

  void loaded(CommitLog::Fragment::Ptr&& frag) override;

  void print(std::ostream& out) {
    out << "BlockLoader(";
    Core::ScopedLock lock(m_mutex);
    out << "blks=" << count_cs_blocks
        << " frags=" << count_fragments
        << " error=" << error
        << " qreq=" << q_req.size()
        << " ack=" << int(m_ack)
        << " m_cs_blks=" << m_cs_blks
        << " logs=" << int(m_logs)
        << " queue=" << m_queue.size()
        << " selected=" << m_f_selected.size()
        << ')';
  }

  private:

  void load_cells();

  class LoadQueue;

  bool                                      m_ack;
  uint8_t                                   m_logs;
  size_t                                    m_cs_blks;

  std::mutex                                m_mutex;
  std::condition_variable                   m_cv;
  std::queue<LoadQueue*>                    m_queue;
  CommitLog::Fragments::Vec                 m_f_selected;
  std::queue<CellStore::Block::Read::Ptr>   m_cs_selected;
  Core::Vector<CellStore::Block::Read::Ptr> m_cs_ready;

};



}}

#endif // swcdb_ranger_db_BlockLoader_h
