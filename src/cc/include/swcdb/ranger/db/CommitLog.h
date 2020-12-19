/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLog_h
#define swcdb_ranger_db_CommitLog_h


#include "swcdb/ranger/db/CommitLogFragment.h"



namespace SWC { namespace Ranger { namespace CommitLog {

class Compact;

class Fragments final : private std::vector<Fragment::Ptr> {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef std::vector<Fragment::Ptr> Vec;
  typedef Fragments*                 Ptr;

  RangePtr            range;
  std::atomic<bool>   stopping;

  explicit Fragments(const DB::Types::KeySeq key_seq);

  void init(const RangePtr& for_range);

  Fragments(const Fragments&) = delete;

  Fragments(const Fragments&&) = delete;
  
  Fragments& operator=(const Fragments&) = delete;

  ~Fragments();

  void schema_update();

  void add(const DB::Cells::Cell& cell);

  void commit_new_fragment(bool finalize=false);

  void add(Fragment::Ptr& frag);

  bool is_compacting() const;

  size_t need_compact(std::vector<Vec>& groups, const Vec& without,
                      size_t vol);
  
  bool try_compact(int tnum = 1);

  void finish_compact(const Compact* compact);

  const std::string get_log_fragment(const int64_t frag) const;

  const std::string get_log_fragment(const std::string& frag) const;

  void load(int &err);
  
  void expand(DB::Cells::Interval& intval);
  
  void expand_and_align(DB::Cells::Interval& intval);

  void load_cells(BlockLoader* loader, bool& is_final, 
                  Vec& frags, uint8_t vol);

  void get(Vec& fragments);

  size_t release(size_t bytes);

  void remove(int &err, Vec& fragments_old, bool safe=false);

  void remove(int &err, Fragment::Ptr& frag, bool remove_file);

  void remove();

  void unload();

  Fragment::Ptr take_ownership(int &err, Fragment::Ptr& frag);

  void take_ownership(int& err, Vec& frags, Vec& removing);

  bool deleting();

  size_t cells_count(bool only_current = false);

  size_t size();

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_encoded();

  bool processing();

  uint64_t next_id();

  void print(std::ostream& out, bool minimal);

  private:

  void _add(Fragment::Ptr& frag);

  uint64_t _next_id();

  bool _need_roll() const;

  size_t _need_compact(std::vector<Vec>& groups,const Vec& without, 
                       size_t vol);

  bool _need_compact_major();

  void _load_cells(BlockLoader* loader, Vec& frags, uint8_t& vol);

  bool _processing() const;

  size_t _size_bytes(bool only_loaded=false);

  size_t _narrow(const DB::Cell::Key& key) const;
  
  std::shared_mutex           m_mutex_cells;
  DB::Cells::MutableVec       m_cells;
  uint8_t                     m_roll_chk;

  Core::StateRunning          m_commit;
  std::atomic<bool>           m_compacting;

  std::shared_mutex           m_mutex;
  bool                        m_deleting;
  std::condition_variable_any m_cv;
  Core::Semaphore             m_sem;
  uint64_t                    m_last_id;
};



}}} // namespace SWC::Ranger::CommitLog


#include "swcdb/ranger/db/CommitLogCompact.h"

#endif // swcdb_ranger_db_CommitLog_h