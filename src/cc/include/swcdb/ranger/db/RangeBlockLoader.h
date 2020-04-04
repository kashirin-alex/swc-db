/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_BlockLoader_h
#define swc_ranger_db_BlockLoader_h

#include "swcdb/ranger/db/RangeBlock.h"
#include "swcdb/ranger/db/CellStoreBlock.h"
#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { 


class BlockLoader final {
  public:
  static const uint8_t MAX_FRAGMENTS = 3;

  Block::Ptr     block;

  BlockLoader(Block::Ptr block);

  ~BlockLoader();

  void run();

  void add(CellStore::Block::Read::Ptr blk);

  void loaded_blk();

  private:

  void load_cellstores();

  void load_cellstores_cells();

  bool check_log();

  void load_log(bool is_final);

  void loaded_frag();
  
  void load_log_cells();

  void completion();
  

  Mutex                                     m_mutex;
  int                                       m_err;
  bool                                      m_processing;
  std::vector<CellStore::Block::Read::Ptr>  m_cs_blocks;
  std::vector<CommitLog::Fragment::Ptr>     m_fragments;
  bool                                      m_chk_cs;
  bool                                      m_checking_log;
  int64_t                                   m_frag_ts;
};



}}

#endif // swc_ranger_db_BlockLoader_h