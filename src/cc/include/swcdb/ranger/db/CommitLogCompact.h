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

    const Time::Measure_ns  ts;
    const uint8_t           worker;
    Fragments::Vec          read_frags;

    Group(Compact* compact, uint8_t worker);

    Group(const Group&) = delete;

    Group(const Group&&) = delete;

    Group& operator=(const Group&) = delete;

    ~Group();

    void run(bool initial);

    void finalize();

    private:

    struct TaskLoadedFrag;
  
    void load();

    void loaded(const Fragment::Ptr& frag);

    void write();

    Core::Atomic<int>                 error;
    Compact*                          compact;
    Core::Atomic<ssize_t>             m_idx;
    Core::Atomic<ssize_t>             m_running;
    Core::Atomic<ssize_t>             m_finishing;
    Core::MutexSptd                   m_mutex;
    DB::Cells::MutableVec             m_cells;
    Fragments::Vec                    m_remove;
    Fragments::Vec                    m_fragments;
  };

  public:

  typedef const std::function<void(const Compact*)> Cb_t;

  Fragments*              log;
  const Time::Measure_ns  ts;
  const uint8_t           preload;
  const uint32_t          repetition;
  size_t                  ngroups;
  size_t                  nfrags;

  Compact(Fragments* log, uint32_t repetition,
          const Fragments::CompactGroups& groups,
          uint8_t cointervaling,
          Compact::Cb_t&& cb = nullptr);

  Compact(const Compact&) = delete;

  Compact(const Compact&&) = delete;

  Compact& operator=(const Compact&) = delete;

  //~Compact() { }

  private:

  void finished(Group* group, size_t cells_count);

  void finalized();

  std::string get_filepath(const int64_t frag) const;

  Core::Atomic<size_t>  m_workers;
  Core::Vector<Group*>  m_groups;
  Cb_t                  m_cb;


};


}}} // namespace SWC::Ranger::CommitLog

#endif // swcdb_ranger_db_CommitLogCompact_h