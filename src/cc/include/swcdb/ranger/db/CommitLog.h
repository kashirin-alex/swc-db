/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLog_h
#define swc_ranger_db_CommitLog_h


#include "swcdb/ranger/db/CommitLogFragment.h"



namespace SWC { namespace Ranger { namespace CommitLog {

class Compact;

class Fragments : private std::vector<Fragment::Ptr> {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef std::vector<Fragment::Ptr> Vec;
  typedef Fragments*                 Ptr;

  static constexpr const uint8_t  MAX_PRELOAD = 2;
  static constexpr const uint8_t  MIN_COMPACT = 3;

  RangePtr            range;
  std::atomic<bool>   stopping;

  explicit Fragments(const Types::KeySeq key_seq);

  void init(RangePtr for_range);

  Fragments(const Fragments&) = delete;

  Fragments(const Fragments&&) = delete;
  
  Fragments& operator=(const Fragments&) = delete;

  ~Fragments();

  void schema_update();

  void add(const DB::Cells::Cell& cell);

  void commit_new_fragment(bool finalize=false);

  void add(Fragment::Ptr frag);

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
                  Fragments::Vec& frags, uint8_t vol);

  void get(Vec& fragments);

  size_t release(size_t bytes);

  void remove(int &err, Vec& fragments_old);

  void remove(int &err, Fragment::Ptr frag, bool remove_file);

  void remove(int &err);

  void unload();

  Fragment::Ptr take_ownership(int &err, Fragment::Ptr frag);

  bool deleting();

  size_t cells_count(bool only_current = false);

  size_t size();

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_encoded();

  bool processing();

  uint64_t next_id();

  std::string to_string();

  private:

  void _add(Fragment::Ptr frag);

  bool _need_roll() const;

  size_t _need_compact(std::vector<Vec>& groups,const Vec& without, 
                       size_t vol);

  bool _need_compact_major();

  void _load_cells(BlockLoader* loader, Fragments::Vec& frags, uint8_t& vol);

  bool _processing() const;

  size_t _size_bytes(bool only_loaded=false);

  std::shared_mutex           m_mutex_cells;
  DB::Cells::MutableVec       m_cells;

  std::shared_mutex           m_mutex;
  bool                        m_commiting;
  bool                        m_deleting;
  std::condition_variable_any m_cv;
  bool                        m_compacting;
  Semaphore                   m_sem;

  using Vec::vector;
};



}}} // namespace SWC::Ranger::CommitLog


#include "swcdb/ranger/db/CommitLogCompact.h"

#endif // swc_ranger_db_CommitLog_h