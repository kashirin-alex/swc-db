/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeBlocks_h
#define swcdb_db_Files_RangeBlocks_h

#include "swcdb/db/Files/RangeBlockLoader.h"

#include "swcdb/db/Files/CellStoreReaders.h"
#include "swcdb/db/Files/CommitLog.h"
#include "swcdb/db/Files/RangeData.h"

namespace SWC { namespace Files { namespace Range {


class Blocks final {
  public:
  typedef Blocks* Ptr;

  // scan >> blk match >> load-cs + load+logs 

  DB::RangeBase::Ptr    range;
  CommitLog::Fragments  commitlog;
  CellStore::Readers    cellstores;

  explicit Blocks();
  
  void init(DB::RangeBase::Ptr for_range);

  Ptr ptr();

  ~Blocks();
  
  void schema_update();

  void processing_increment();

  void processing_decrement();

  void load(int& err);

  void unload();
  
  void remove(int& err);
  
  void apply_new(int &err,
                CellStore::Writers& w_cellstores, 
                std::vector<CommitLog::Fragment::Ptr>& fragments_old);

  void add_logged(const DB::Cells::Cell& cell, bool& intval_chg);

  void scan(DB::Cells::ReqScan::Ptr req, Block::Ptr blk_ptr = nullptr);

  void split(Block::Ptr blk, bool loaded=true);

  const bool _split(Block::Ptr blk, bool loaded=true);

  const size_t cells_count();

  const size_t size();

  const size_t size_bytes();

  const size_t size_bytes_total(bool only_loaded=false);

  void release_prior(Block::Ptr ptr);

  /*
  void release_and_merge(Block::Ptr ptr);
  */

  const size_t release(size_t bytes=0);

  const bool processing();

  void wait_processing();

  const std::string to_string();

  private:

  const size_t _size();

  const size_t _size_bytes();
  
  const bool _processing() const ;

  void _clear();

  void init_blocks(int& err);

  std::shared_mutex   m_mutex;
  Block::Ptr          m_block;
  std::atomic<size_t> m_processing;

};





}}}



//#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Files/RangeBlock.cc"
#include "swcdb/db/Files/RangeBlocks.cc"
#include "swcdb/db/Files/RangeBlockLoader.cc"
//#endif 

#endif // swcdb_db_Files_RangeBlocks_h