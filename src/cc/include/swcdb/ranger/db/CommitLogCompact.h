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

    private:

    void load_more();

    void loaded(Fragment::Ptr frag);

    void load();

    void write();

    void finalize();
    
    std::atomic<int>                  error;
    Compact*                          compact;
    size_t                            read_idx;
    DB::Cells::MutableVec             m_cells;
    QueueSafeStated<Fragment::Ptr>    m_queue;
    std::vector<Fragment::Ptr>        m_remove;
    std::vector<Fragment::Ptr>        m_fragments;
    Semaphore                         m_sem;
    std::atomic<uint8_t>              m_loading;
  };

  public:

  Fragments*            log;
  const Types::KeySeq   key_seq;
  const uint64_t        ts;
  const uint32_t        repetition;
  const uint32_t        nfrags;
  const uint8_t         process_state;

  Compact(Fragments* log, const Types::KeySeq key_seq,
          int tnum, const std::vector<Fragment::Ptr>& frags, 
          uint8_t process_state);

  ~Compact();

  private:

  void applying();

  void finished(Group* group);

  const std::string get_filepath(const int64_t frag) const;

  std::mutex            m_mutex;
  uint8_t               m_workers;
  uint8_t               m_applying;
  std::vector<Group*>   m_groups;


};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogCompact_h