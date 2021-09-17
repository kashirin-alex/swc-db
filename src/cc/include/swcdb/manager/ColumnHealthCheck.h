/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_ColumnHealthCheck_h
#define swcdb_manager_ColumnHealthCheck_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"


namespace SWC { namespace Manager {


class ColumnHealthCheck final
      : public std::enable_shared_from_this<ColumnHealthCheck> {
  public:

  typedef std::shared_ptr<ColumnHealthCheck>  Ptr;
  const Column::Ptr                           col;
  const int64_t                               check_ts;
  const int32_t                               check_intval;
  Core::CompletionCounter<uint64_t>           completion;


  class RangerCheck final
      : public std::enable_shared_from_this<RangerCheck> {
    public:

    typedef std::shared_ptr<RangerCheck>  Ptr;
    const ColumnHealthCheck::Ptr          col_checker;
    const Ranger::Ptr                     rgr;

    RangerCheck(const ColumnHealthCheck::Ptr& col_checker,
                const Ranger::Ptr& rgr);

    ~RangerCheck() noexcept;

    void add_range(const Range::Ptr& range);

    bool add_ranges(uint8_t more);

    void handle(const Range::Ptr& range, int err, uint8_t flags);

    private:

    void _add_range(const Range::Ptr& range);

    Core::MutexSptd           m_mutex;
    std::queue<Range::Ptr>    m_ranges;
    uint8_t                   m_checkings;

    Core::Atomic<size_t>      m_success;
    Core::Atomic<size_t>      m_failures;
  };



  class ColumnMerger final
      : public std::enable_shared_from_this<ColumnMerger> {
    public:

    typedef std::shared_ptr<ColumnMerger>   Ptr;
    const ColumnHealthCheck::Ptr            col_checker;
    Core::Vector<Range::Ptr>                m_ranges;
    DB::Cells::Result                       cells;

    ColumnMerger(const ColumnHealthCheck::Ptr& col_checker,
                 Core::Vector<Range::Ptr>&& ranges) noexcept;

    ~ColumnMerger() noexcept { }

    void run_master();

    void run();

    void completion();

    class RangesMerger final
        : public std::enable_shared_from_this<RangesMerger> {
      public:

      typedef std::shared_ptr<RangesMerger>   Ptr;
      const ColumnMerger::Ptr                 col_merger;

      RangesMerger(const ColumnMerger::Ptr& col_merger,
                   Core::Vector<Range::Ptr>&& ranges) noexcept;

      ~RangesMerger() noexcept { }

      void run();

      void handle(const Range::Ptr& range, int err, bool empty);

      private:
      Core::MutexSptd           m_mutex;
      int                       m_err;
      Core::Vector<Range::Ptr>  m_ranges;
      Core::Vector<Range::Ptr>  m_ready;
    };

    private:
    Core::MutexSptd                 m_mutex;
    Core::Vector<RangesMerger::Ptr> m_mergers;
  };



  ColumnHealthCheck(const Column::Ptr& col,
                    int64_t check_ts, uint32_t check_intval);

  ~ColumnHealthCheck() noexcept { }

  void run(bool initial=true);

  void add_mergeable(const Range::Ptr& range);

  void finishing(bool finished_range);

  private:
  Core::StateRunning              m_check;
  Core::MutexSptd                 m_mutex;
  Core::Vector<RangerCheck::Ptr>  m_checkers;
  Core::Vector<Range::Ptr>        m_mergeable_ranges;

};


typedef Core::Vector<ColumnHealthCheck::Ptr> ColumnHealthChecks;


}}

#endif // swcdb_manager_ColumnHealthCheck_h
