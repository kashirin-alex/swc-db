/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLog_h
#define swcdb_ranger_db_CommitLog_h


#include "swcdb/ranger/db/CommitLogFragment.h"



namespace SWC { namespace Ranger { namespace CommitLog {

class Compact;

class Fragments final : private Core::Vector<Fragment::Ptr> {

  /* file-format(dir-structure):
    ../log/{N}.frag
  */

  public:

  typedef Core::Vector<Fragment::Ptr> Vec;
  typedef Fragments*                  Ptr;
  typedef Core::Vector<Vec>           CompactGroups;

  RangePtr            range;
  Core::AtomicBool    stopping;

  explicit Fragments(const DB::Types::KeySeq key_seq);

  void init(const RangePtr& for_range);

  Fragments(const Fragments&) = delete;

  Fragments(const Fragments&&) = delete;

  Fragments& operator=(const Fragments&) = delete;

  ~Fragments() noexcept;

  void schema_update();

  void add(const DB::Cells::Cell& cell);

  void commit() noexcept;

  size_t commit_release();

  void commit_finalize();

  void add(Fragment::Ptr& frag);

  bool is_compacting() const;

  size_t need_compact(CompactGroups& groups, const Vec& without,
                      size_t vol);

  bool try_compact(uint32_t tnum = 1);

  void finish_compact(const Compact* compact);

  std::string get_log_fragment(const int64_t frag) const;

  std::string get_log_fragment(const std::string& frag) const;

  void load(int &err);

  void expand(DB::Cells::Interval& intval);

  void expand_and_align(DB::Cells::Interval& intval);

  void load_cells(BlockLoader* loader, bool& is_final,
                  Vec& frags, uint8_t vol);

  void get(Vec& fragments);

  size_t release(size_t bytes);

  void remove(int &err, Vec& fragments_old);

  void remove(int &err, Fragment::Ptr& frag, bool remove_file);

  void remove();

  void unload();

  Fragment::Ptr take_ownership(int &err, Fragment::Ptr& frag);

  void take_ownership(int& err, Vec& frags, Vec& removing);

  bool deleting();

  size_t cells_count(bool only_current = false);

  bool empty();

  size_t size() noexcept;

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_encoded();

  bool processing() noexcept;

  uint64_t next_id();

  void print(std::ostream& out, bool minimal);

  private:

  size_t _commit(bool finalize);

  void _add(Fragment::Ptr& frag);

  uint64_t _next_id();

  void _remove(int &err, Vec& fragments_old, Core::Semaphore* semp);

  bool _need_roll() const noexcept;

  size_t _need_compact(CompactGroups& groups,const Vec& without,
                       size_t vol);

  bool _need_compact_major();

  void _load_cells(BlockLoader* loader, Vec& frags, uint8_t& vol);

  bool _processing() const noexcept;

  size_t _size_bytes(bool only_loaded=false);

  size_t _narrow(const DB::Cell::Key& key) const;

  std::shared_mutex           m_mutex_cells;
  DB::Cells::MutableVec       m_cells;

  Core::StateRunning          m_commit;
  Core::AtomicBool            m_compacting;

  std::shared_mutex           m_mutex;
  bool                        m_deleting;
  std::condition_variable_any m_cv;
  Core::Semaphore             m_sem;
  uint64_t                    m_last_id;
  Core::Atomic<size_t>        m_releasable_bytes;
};



}}} // namespace SWC::Ranger::CommitLog


#include "swcdb/ranger/db/CommitLogCompact.h"

#endif // swcdb_ranger_db_CommitLog_h