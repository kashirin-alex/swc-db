/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_RangeBlocks_h
#define swc_ranger_db_RangeBlocks_h

namespace SWC { namespace Ranger { namespace CommitLog {
class Fragments;
typedef Fragments*  FragmentsPtr;
}}}

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

  explicit Blocks(const Types::KeySeq key_seq);

  Blocks(const Blocks&) = delete;

  Blocks(const Blocks&&) = delete;

  Blocks& operator=(const Blocks&) = delete;

  void init(const RangePtr& for_range);

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
                CommitLog::Fragments::Vec& fragments_old);

  void add_logged(const DB::Cells::Cell& cell);

  void scan(ReqScan::Ptr req, Block::Ptr blk_ptr = nullptr);

  bool _split(Block::Ptr blk, bool loaded=true);

  size_t cells_count();

  size_t size();

  size_t size_bytes();

  size_t size_bytes_total(bool only_loaded=false);

  size_t release(size_t bytes=0);

  bool processing();

  void wait_processing();

  void print(std::ostream& out, bool minimal);

  private:

  size_t _size();

  size_t _size_bytes();
  
  bool _processing() const ;

  void _clear();

  void init_blocks(int& err);

  size_t _get_block_idx(const Block::Ptr blk) const;

  size_t _narrow(const DB::Cell::Key& key) const;

  static const uint8_t MAX_IDX_NARROW = 20;

  Mutex                     m_mutex;
  Block::Ptr                m_block;
  std::vector<Block::Ptr>   m_blocks_idx;
  std::atomic<size_t>       m_processing;
  

};





}}



#endif // swc_ranger_db_RangeBlocks_h