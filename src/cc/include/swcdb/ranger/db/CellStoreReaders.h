/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CellStoreReaders_h
#define swcdb_ranger_db_CellStoreReaders_h

#include "swcdb/ranger/db/CellStore.h"


namespace SWC { namespace Ranger { namespace CellStore {

class Readers final : private Core::Vector<Read::Ptr> {
  public:

  typedef Core::Vector<Read::Ptr> Vec;
  typedef Readers*                Ptr;

  using Vec::empty;
  using Vec::size;
  using Vec::begin;
  using Vec::end;
  using Vec::cbegin;
  using Vec::cend;
  using Vec::front;
  using Vec::back;
  using Vec::erase;
  using Vec::reserve;

  RangePtr range;

  explicit Readers() noexcept { }

  Readers(const Readers&) = delete;

  Readers(const Readers&&) = delete;

  Readers& operator=(const Readers&) = delete;

  void init(const RangePtr& for_range) {
    range = for_range;
  }

  ~Readers() noexcept { }

  void add(Read::Ptr cs) {
    push_back(cs);
  }

  void load(int& err);

  void expand(DB::Cells::Interval& intval) const;

  void expand_and_align(DB::Cells::Interval& intval) const;

  size_t size_bytes(bool only_loaded=false) const;

  uint32_t get_cell_revs() const;

  int64_t get_ts_earliest() const;

  size_t blocks_count() const;

  size_t release(size_t bytes);

  bool processing() const noexcept;

  void remove(int &err);

  void unload();

  void clear();

  void load_cells(BlockLoader* loader);

  void get_blocks(int& err, Read::Blocks& to) const;

  void get_prev_key_end(uint32_t idx, DB::Cell::Key& key) const;

  void get_key_end(DB::Cell::Key& key) const;

  bool need_compaction(size_t cs_sz, size_t blk_size) const;

  uint32_t encoded_length() const;

  void encode(uint8_t** ptr) const;

  void decode(int &err, const uint8_t** ptr, size_t* remain);

  void load_from_path(int &err);

  void replace(int &err, Writers& w_cellstores);

  void move_from(int &err, Readers::Vec& mv_css);

  void print(std::ostream& out, bool minimal=true) const;

  private:

  void _free();

  void _close();

};


}}} // namespace SWC::Ranger::CellStore

#endif // swcdb_ranger_db_CellStoreReaders_h