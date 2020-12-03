/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_BlockLoader_h
#define swcdb_ranger_db_BlockLoader_h

#include "swcdb/ranger/db/RangeBlock.h"
#include "swcdb/ranger/db/CellStoreBlock.h"
#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { 


class BlockLoader final {
  public:

  Block::Ptr          block;
  std::atomic<size_t> count_cs_blocks;
  std::atomic<size_t> count_fragments;
  std::atomic<int>    error;

  explicit BlockLoader(Block::Ptr block);

  BlockLoader(const BlockLoader&) = delete;

  BlockLoader(const BlockLoader&&) = delete;

  BlockLoader& operator=(const BlockLoader&) = delete;

  ~BlockLoader();

  void run(bool preloading);

  void add(CellStore::Block::Read::Ptr blk);

  void load_log();

  void loaded_blk();

  private:

  void load_cs_blocks();

  void load_fragments();

  void _load_fragments();

  Core::Semaphore                         m_sem_cs;
  Core::Semaphore                         m_sem_log;
  Core::MutexSptd                         m_mutex;
  std::queue<CellStore::Block::Read::Ptr> m_cs_blocks;
  std::queue<CommitLog::Fragment::Ptr>    m_fragments;
  bool                                    m_cs_blocks_running;
  bool                                    m_fragments_running;

};



}}

#endif // swcdb_ranger_db_BlockLoader_h