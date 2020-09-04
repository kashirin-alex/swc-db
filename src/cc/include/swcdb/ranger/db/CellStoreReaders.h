/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CellStoreReaders_h
#define swc_ranger_db_CellStoreReaders_h

#include "swcdb/ranger/db/CellStore.h"


namespace SWC { namespace Ranger { namespace CellStore {

class Readers final : private std::vector<Read::Ptr> {
  public:

  typedef std::vector<Read::Ptr>  Vec;
  typedef Readers*                Ptr;
  
  using Vec::empty;
  using Vec::size;

  RangePtr range;

  explicit Readers();

  Readers(const Readers&) = delete;

  Readers(const Readers&&) = delete;
  
  Readers& operator=(const Readers&) = delete;

  void init(const RangePtr& for_range);

  ~Readers();

  void add(Read::Ptr cs);

  void load(int& err);

  void expand(DB::Cells::Interval& intval) const;

  void expand_and_align(DB::Cells::Interval& intval) const;

  size_t size_bytes(bool only_loaded=false) const;

  uint32_t get_cell_revs() const;
  
  int64_t get_ts_earliest() const;

  size_t blocks_count() const;

  size_t release(size_t bytes);

  bool processing() const;
  
  void remove(int &err);

  void unload();

  void clear();
  
  void load_cells(BlockLoader* loader);
  
  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to) const;

  void get_prev_key_end(uint32_t idx, DB::Cell::Key& key) const;

  void get_key_end(DB::Cell::Key& key) const;

  bool need_compaction(size_t cs_sz, size_t blk_size) const;

  size_t encoded_length() const;

  void encode(uint8_t** ptr) const;
  
  void decode(int &err, const uint8_t** ptr, size_t* remain);
  
  void load_from_path(int &err);

  void replace(int &err, CellStore::Writers& w_cellstores);

  void print(std::ostream& out, bool minimal=true) const;

  private:
  
  void _free();

  void _close();

};


}}} // namespace SWC::Ranger::CellStore

#endif // swc_ranger_db_CellStoreReaders_h