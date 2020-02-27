/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_BlockLoader_h
#define swcdb_db_Files_BlockLoader_h

#include "swcdb/db/Files/RangeBlock.h"
#include "swcdb/db/Files/CellStoreBlock.h"
#include "swcdb/db/Files/CommitLogFragment.h"

namespace SWC { namespace Files { namespace Range {


class BlockLoader final {
  public:
  static const uint8_t MAX_FRAGMENTS = 3;

  Range::Block::Ptr     block;

  BlockLoader(Range::Block::Ptr block);

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
  

  std::mutex                                m_mutex;
  int                                       m_err;
  bool                                      m_processing;
  std::vector<CellStore::Block::Read::Ptr>  m_cs_blocks;
  std::vector<CommitLog::Fragment::Ptr>     m_fragments;
  bool                                      m_chk_cs;
  bool                                      m_checking_log;
  int64_t                                   m_frag_ts;
};



}}}

#endif