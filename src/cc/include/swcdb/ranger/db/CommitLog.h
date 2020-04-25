/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLog_h
#define swc_ranger_db_CommitLog_h

#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { namespace CommitLog {


class Fragments final {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef Fragments*  Ptr;

  static constexpr const uint8_t  MAX_PRELOAD = 3;
  static constexpr const uint8_t  MAX_COMPACT = 6;

  std::atomic<bool>    compacting;
  RangePtr             range;

  explicit Fragments(const Types::KeySeq key_seq);

  void init(RangePtr for_range);

  ~Fragments();

  void schema_update();

  void add(const DB::Cells::Cell& cell);

  void commit_new_fragment(bool finalize=false);

  const std::string get_log_fragment(const int64_t frag) const;

  const std::string get_log_fragment(const std::string& frag) const;

  void load(int &err);
  
  void expand(DB::Cells::Interval& intval);
  
  void expand_and_align(DB::Cells::Interval& intval);

  void load_cells(BlockLoader* loader, bool final, int64_t after_ts,
                  std::vector<Fragment::Ptr>& fragments);

  void load_cells(BlockLoader* loader);

  void need_compact(std::vector<Fragment::Ptr>& fragments,
                    const std::vector<Fragment::Ptr>& without);

  void get(std::vector<Fragment::Ptr>& fragments);

  size_t release(size_t bytes);

  void remove(int &err, std::vector<Fragment::Ptr>& fragments_old);

  void remove(int &err, Fragment::Ptr frag, bool remove_file);

  void remove(int &err);

  void unload();

  void take_ownership(int &err, Fragment::Ptr frag);

  bool deleting();

  size_t cells_count(bool only_current = false);

  size_t size();

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_encoded();

  bool processing();

  std::string to_string();

  private:

  bool _need_roll() const;

  bool _processing() const;

  size_t _size_bytes(bool only_loaded=false);

  std::shared_mutex           m_mutex_cells;
  DB::Cells::Mutable          m_cells;

  std::shared_mutex           m_mutex;
  bool                        m_commiting;
  bool                        m_deleting;
  std::condition_variable_any m_cv;
  std::vector<Fragment::Ptr>  m_fragments;

};



}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLog_h