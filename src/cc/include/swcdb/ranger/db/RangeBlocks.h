/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_RangeBlocks_h
#define swcdb_ranger_db_RangeBlocks_h

namespace SWC { namespace Ranger { namespace CommitLog {
class Fragments;
typedef Fragments*  FragmentsPtr;
}}}

#include "swcdb/ranger/db/RangeBlockLoader.h"
#ifdef SWC_RANGER_WITH_RANGEDATA
#include "swcdb/ranger/db/RangeData.h"
#endif

namespace SWC { namespace Ranger {


class Blocks final {
  public:
  typedef Blocks* Ptr;

  // scan >> blk match >> load-cs + load+logs

  RangePtr              range;
  CommitLog::Fragments  commitlog;
  CellStore::Readers    cellstores;

  explicit Blocks(const DB::Types::KeySeq key_seq);

  Blocks(const Blocks&) = delete;

  Blocks(const Blocks&&) = delete;

  Blocks& operator=(const Blocks&) = delete;

  void init(const RangePtr& for_range);

  Ptr ptr();

  ~Blocks() noexcept { }

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

  bool _split(Block::Ptr blk, bool loaded);

  size_t cells_count();

  size_t size() noexcept;

  size_t size_bytes();

  size_t size_bytes_total(bool only_loaded=false);

  size_t release(size_t bytes);

  size_t release(size_t bytes, uint8_t level);

  void reset_blocks();

  bool processing() noexcept;

  bool wait_processing(int64_t quit_time=0);

  void print(std::ostream& out, bool minimal);

  private:

  size_t SWC_PURE_FUNC _size() const noexcept;

  size_t _size_bytes();

  bool _processing() const noexcept;

  void _clear();

  void init_blocks(int& err);

  size_t SWC_PURE_FUNC _get_block_idx(const Block::Ptr blk) const noexcept;

  size_t _narrow(const DB::Cell::Key& key) const;

  static const uint8_t MAX_IDX_NARROW = 20;

  Core::MutexSptd           m_mutex;
  Block::Ptr                m_block;
  Core::Vector<Block::Ptr>  m_blocks_idx;
  Core::Atomic<size_t>      m_processing;


};





}}



#endif // swcdb_ranger_db_RangeBlocks_h