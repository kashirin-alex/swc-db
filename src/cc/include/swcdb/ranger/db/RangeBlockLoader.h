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

  Block::Ptr           block;
  Core::Atomic<size_t> count_cs_blocks;
  Core::Atomic<size_t> count_fragments;

  struct ReqQueue {
    ReqScan::Ptr  req;
    const int64_t ts;
    ~ReqQueue() noexcept { }
  };
  std::queue<ReqQueue> q_req; // synced by Block mutex
  Core::Atomic<int>    error;
  const uint8_t        preload;


  explicit BlockLoader(Block::Ptr block);

  BlockLoader(const BlockLoader&) = delete;

  BlockLoader(const BlockLoader&&) = delete;

  BlockLoader& operator=(const BlockLoader&) = delete;

  ~BlockLoader() noexcept { }

  void add(const ReqScan::Ptr& req);

  void run();

  void add(CellStore::Block::Read::Ptr blk);

  void loaded_blk();

  void loaded(CommitLog::Fragment::Ptr&& frag) override;

  private:

  void load_cellstores_cells();

  void load_log(bool is_final, bool is_more=false);

  void load_log_cells();

  void completion();


  Core::StateRunning                        m_check_log;
  bool                                      m_processing;
  uint8_t                                   m_logs;
  Core::MutexSptd                           m_mutex;

  std::queue<CellStore::Block::Read::Ptr>   m_cs_blocks;
  CommitLog::Fragments::Vec                 m_f_selected;
  std::queue<CommitLog::Fragment::Ptr>      m_fragments;

};



}}

#endif // swcdb_ranger_db_BlockLoader_h
