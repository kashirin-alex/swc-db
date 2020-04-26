/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLogCompact_h
#define swc_ranger_db_CommitLogCompact_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Compact final {
  public:

  std::atomic<bool>  stop;
  std::atomic<bool>  error;

  Compact(Fragments* log, const Types::KeySeq key_seq);

  ~Compact();

  void run(int tnum, const std::vector<Fragment::Ptr>& frags);

  private:

  void loaded();

  void load();

  void write(bool last);

  void finalize();

  const std::string get_filepath(const int64_t frag) const;

  Fragments*                        log;
  std::mutex                        m_mutex;
  DB::Cells::Mutable                m_cells;
  std::condition_variable           m_cv;
  QueueSafeStated<Fragment::Ptr>    m_queue;
  size_t                            m_remain;
  std::vector<Fragment::Ptr>        m_remove;
  std::vector<Fragment::Ptr>        m_fragments;
  Semaphore                         m_sem;

};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogCompact_h