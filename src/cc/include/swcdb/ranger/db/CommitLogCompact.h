/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLogCompact_h
#define swcdb_ranger_db_CommitLogCompact_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Compact final {
    
  class Group final {
    public:

    const uint64_t              ts;
    const uint8_t               worker;
    Fragments::Vec              read_frags;
    
    Group(Compact* compact, uint8_t worker);

    Group(const Group&) = delete;

    Group(const Group&&) = delete;
  
    Group& operator=(const Group&) = delete;

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
    Core::MutexAtomic                 m_mutex;
    size_t                            m_read_idx;
    uint32_t                          m_loading;
    uint32_t                          m_processed;
    DB::Cells::MutableVec             m_cells;
    Core::QueueSafe<Fragment::Ptr>    m_queue;
    Fragments::Vec                    m_remove;
    Fragments::Vec                    m_fragments;
    Core::Semaphore                   m_sem;
  };

  public:

  typedef const std::function<void(const Compact*)> Cb_t;

  Fragments*            log;
  const uint64_t        ts;
  const size_t          repetition;
  size_t                ngroups;
  size_t                nfrags;

  Compact(Fragments* log, int repetition, 
          const std::vector<Fragments::Vec>& groups, Cb_t& cb = 0);

  Compact(const Compact&) = delete;

  Compact(const Compact&&) = delete;
  
  Compact& operator=(const Compact&) = delete;

  ~Compact();

  private:

  void finished(Group* group, size_t cells_count);

  const std::string get_filepath(const int64_t frag) const;

  Core::MutexAtomic     m_mutex;
  uint8_t               m_workers;
  std::vector<Group*>   m_groups;
  Cb_t                  m_cb;


};


}}} // namespace SWC::Ranger::CommitLog

#endif // swcdb_ranger_db_CommitLogCompact_h