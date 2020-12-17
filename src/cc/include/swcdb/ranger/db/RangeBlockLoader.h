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


class BlockLoader final {
  public:

  Block::Ptr          block;
  const uint8_t       preload;
  std::atomic<size_t> count_cs_blocks;
  std::atomic<size_t> count_fragments;
  std::atomic<int>    error;

  struct ReqQueue {
    ReqScan::Ptr  req;
    const int64_t ts;
  };
  std::queue<ReqQueue> q_req; // synced by Block mutex


  explicit BlockLoader(Block::Ptr block);

  BlockLoader(const BlockLoader&) = delete;

  BlockLoader(const BlockLoader&&) = delete;

  BlockLoader& operator=(const BlockLoader&) = delete;

  ~BlockLoader();

  void add(const ReqScan::Ptr& req);

  void run();

  void add(CellStore::Block::Read::Ptr blk);

  void loaded_blk();

  private:

  void load_cellstores_cells();

  bool check_log();

  void load_log(bool is_final, bool is_more=false);

  void loaded_frag(const CommitLog::Fragment::Ptr& frag);
  
  void load_log_cells();

  void completion();


  Core::MutexSptd                           m_mutex;
  bool                                      m_processing;
  bool                                      m_checking_log;
  uint8_t                                   m_logs;

  std::queue<CellStore::Block::Read::Ptr>   m_cs_blocks;
  std::vector<CommitLog::Fragment::Ptr>     m_f_selected;
  std::queue<CommitLog::Fragment::Ptr>      m_fragments;

};



}}

#endif // swcdb_ranger_db_BlockLoader_h
