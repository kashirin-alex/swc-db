/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLogCompact_h
#define swc_ranger_db_CommitLogCompact_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Compact final {
    
  class Group final {
    public:

    const uint64_t              ts;
    const uint8_t               worker;
    std::vector<Fragment::Ptr>  read_frags;
    
    Group(Compact* compact, uint8_t worker);

    ~Group();

    void run();

    void finalize();

    private:

    void load_more();

    void loaded(Fragment::Ptr frag);

    void load();

    void write();
    
    std::atomic<int>                  error;
    Compact*                          compact;
    LockAtomic::Unique                m_mutex;
    size_t                            m_read_idx;
    uint32_t                          m_loading;
    uint32_t                          m_processed;
    DB::Cells::MutableVec             m_cells;
    QueueSafe<Fragment::Ptr>          m_queue;
    std::vector<Fragment::Ptr>        m_remove;
    std::vector<Fragment::Ptr>        m_fragments;
    Semaphore                         m_sem;
  };

  public:

  typedef const std::function<void(const Compact*)> Cb_t;

  Fragments*            log;
  const uint64_t        ts;
  const size_t          repetition;
  size_t                ngroups;
  size_t                nfrags;

  Compact(Fragments* log, int repetition, 
          const std::vector<std::vector<Fragment::Ptr>>& groups, 
          Cb_t& cb = 0);

  ~Compact();

  private:

  void finished(Group* group);

  const std::string get_filepath(const int64_t frag) const;

  LockAtomic::Unique    m_mutex;
  uint8_t               m_workers;
  std::vector<Group*>   m_groups;
  Cb_t                  m_cb;


};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogCompact_h