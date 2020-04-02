/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CellStoreReaders_h
#define swc_ranger_db_CellStoreReaders_h

#include "swcdb/ranger/db/CellStore.h"


namespace SWC { namespace Ranger { namespace CellStore {

class Readers final {
  public:
  typedef Readers*  Ptr;

  RangePtr range;

  explicit Readers();

  void init(RangePtr for_range);

  ~Readers();

  void add(Read::Ptr cs);

  void load(int& err);

  void expand(DB::Cells::Interval& intval);

  void expand_and_align(DB::Cells::Interval& intval);

  bool empty();

  size_t size();

  size_t size_bytes(bool only_loaded=false);

  size_t blocks_count();

  size_t release(size_t bytes);

  bool processing();
  
  void remove(int &err);

  void unload();

  void clear();
  
  void load_cells(BlockLoader* loader);
  
  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to);

  void get_prev_key_end(uint32_t idx, DB::Cell::Key& key);

  bool need_compaction(size_t cs_sz, size_t blk_size);

  size_t encoded_length();

  void encode(uint8_t** ptr);
  
  void decode(int &err, const uint8_t** ptr, size_t* remain);
  
  void load_from_path(int &err);

  void replace(int &err, CellStore::Writers& w_cellstores);

  std::string to_string();

  private:
  
  void _free();

  void _close();

  bool _processing();
  
  size_t _size_bytes(bool only_loaded=false);

  //std::shared_mutex       m_mutex;
  std::vector<Read::Ptr>  m_cellstores;
};


}}} // namespace SWC::Ranger::CellStore

#endif // swc_ranger_db_CellStoreReaders_h