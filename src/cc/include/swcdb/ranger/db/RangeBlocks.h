/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_ranger_db_RangeBlocks_h
#define swc_ranger_db_RangeBlocks_h

#include "swcdb/ranger/db/RangeBlockLoader.h"

#include "swcdb/ranger/db/CellStoreReaders.h"
#include "swcdb/ranger/db/CommitLog.h"
#include "swcdb/ranger/db/RangeData.h"

namespace SWC { namespace Ranger { 


class Blocks final {
  public:
  typedef Blocks* Ptr;

  // scan >> blk match >> load-cs + load+logs 

  RangePtr              range;
  CommitLog::Fragments  commitlog;
  CellStore::Readers    cellstores;

  explicit Blocks();
  
  void init(RangePtr for_range);

  Ptr ptr();

  ~Blocks();
  
  void schema_update();

  void processing_increment();

  void processing_decrement();

  void load(int& err);

  void unload();
  
  void remove(int& err);
  
  void expand(DB::Cells::Interval& intval);

  void expand_and_align(DB::Cells::Interval& intval);

  void apply_new(int &err,
                CellStore::Writers& w_cellstores, 
                std::vector<CommitLog::Fragment::Ptr>& fragments_old);

  void add_logged(const DB::Cells::Cell& cell);

  void scan(ReqScan::Ptr req, Block::Ptr blk_ptr = nullptr);

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

  size_t _get_block_idx(const Block::Ptr blk) const;

  size_t _narrow(const DB::Cell::Key& key) const;

  static const uint8_t MAX_IDX_NARROW = 20;

  std::shared_mutex         m_mutex;
  Block::Ptr                m_block;
  std::vector<Block::Ptr>   m_blocks_idx;
  std::atomic<size_t>       m_processing;
  

};





}}



#endif // swc_ranger_db_RangeBlocks_h