/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_RangeBlock_h
#define swcdb_ranger_db_RangeBlock_h


#include "swcdb/fs/Interface.h"
#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/Cells/MutableVec.h"


namespace SWC { namespace Ranger {

 //Forawrd Declaration
class Blocks;
class BlockLoader;


class Block final {
  public:

  typedef Block* Ptr;

  enum State : uint8_t {
    NONE    = 0x00,
    LOADING = 0x01,
    LOADED  = 0x02
  };

  enum ScanState : uint8_t {
    UKNOWN    = 0x00,
    QUEUED    = 0x01,
    RESPONDED = 0x02,
    SYNCED    = 0x03
  };

  Blocks*     blocks;
  Block::Ptr  next;
  Block::Ptr  prev;


  static Ptr make(const DB::Cells::Interval& interval,
                  Blocks* blocks,
                  State state=State::NONE);

  explicit Block(const DB::Cells::Interval& interval,
                 Blocks* blocks, State state=State::NONE);

  Block(const Block&) = delete;

  Block(const Block&&) = delete;

  Block& operator=(const Block&) = delete;

  ~Block();

  size_t _releasing_size() const noexcept;

  Ptr ptr();

  void schema_update() noexcept;

  void _set_prev_key_end(const DB::Cell::Key& key);

  void _set_prev_key_end(const Ptr blk);

  Condition::Comp _cond_key_end(const DB::Cell::Key& key) const;

  void _set_key_end(const DB::Cell::Key& key);

  void _free_key_end();

  bool is_consist(const DB::Cells::Interval& intval) const;

  bool is_in_end(const DB::Cell::Key& key) const;

  bool _is_in_end(const DB::Cell::Key& key) const;

  bool is_next(const DB::Specs::Interval& spec) const;

  bool includes(const DB::Specs::Interval& spec) const;

  bool _includes_begin(const DB::Specs::Interval& spec) const;

  bool includes_end(const DB::Specs::Interval& spec) const;

  void preload();

  bool add_logged(const DB::Cells::Cell& cell);

  void load_final(const DB::Cells::MutableVec& cells);

  size_t load_cells(const uint8_t* buf, size_t remain,
                    uint32_t revs, size_t avail,
                    bool& was_splitted, bool synced=false);

  bool splitter();

  ScanState scan(const ReqScan::Ptr& req);

  void loader_loaded();

  Ptr split(bool loaded);

  Ptr _split(bool loaded);

  void _add(Ptr blk);

  size_t release();

  void processing_increment() noexcept;

  void processing_decrement() noexcept;

  bool loaded() const noexcept;

  bool need_load() const noexcept;

  bool processing() noexcept;

  size_t size();

  size_t _size() const noexcept;

  size_t size_bytes();

  size_t size_of_internal();

  bool _need_split() const noexcept;

  void print(std::ostream& out);

  private:

  ScanState _scan(const ReqScan::Ptr& req, bool synced=false);


  std::shared_mutex           m_mutex;
  DB::Cells::Mutable          m_cells;
  Core::Atomic<size_t>        m_releasable_bytes;

  mutable Core::MutexAtomic   m_mutex_intval;
  uint32_t                    m_split_rev;
  DB::Cell::Key               m_prev_key_end;
  DB::Cell::Key               m_key_end;

  Core::MutexSptd             m_mutex_state;
  Core::Atomic<size_t>        m_processing;
  Core::Atomic<State>         m_state;
  BlockLoader*                m_loader;

};






}}

#endif // swcdb_ranger_db_RangeBlock_h