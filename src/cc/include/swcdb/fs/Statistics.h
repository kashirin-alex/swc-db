/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Statistics_h
#define swcdb_fs_Statistics_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/MutexAtomic.h"


namespace SWC { namespace FS {


struct Statistics {


  enum Command : uint8_t {
    OPEN_SYNC,
    OPEN_ASYNC,
    CREATE_SYNC,
    CREATE_ASYNC,
    CLOSE_SYNC,
    CLOSE_ASYNC,
    READ_SYNC,
    READ_ASYNC,
    APPEND_SYNC,
    APPEND_ASYNC,
    SEEK_SYNC,
    SEEK_ASYNC,
    REMOVE_SYNC,
    REMOVE_ASYNC,
    LENGTH_SYNC,
    LENGTH_ASYNC,
    PREAD_SYNC,
    PREAD_ASYNC,
    MKDIRS_SYNC,
    MKDIRS_ASYNC,
    FLUSH_SYNC,
    FLUSH_ASYNC,
    RMDIR_SYNC,
    RMDIR_ASYNC,
    READDIR_SYNC,
    READDIR_ASYNC,
    EXISTS_SYNC,
    EXISTS_ASYNC,
    RENAME_SYNC,
    RENAME_ASYNC,
    SYNC_SYNC,
    SYNC_ASYNC,
    WRITE_SYNC,
    WRITE_ASYNC,
    READ_ALL_SYNC,
    READ_ALL_ASYNC,
    COMBI_PREAD_SYNC,
    COMBI_PREAD_ASYNC,

    MAX
  };

  static const char* to_string(Command cmd) noexcept;


  struct Metric : Core::MutexAtomic {

    struct Tracker : Time::Measure_ns {
      SWC_CAN_INLINE
      Tracker() noexcept
              : Time::Measure_ns(Time::Measure_ns::duration::zero()),
                m(nullptr) {
      }
      SWC_CAN_INLINE
      Tracker(Metric* m) noexcept : m(m) { }
      void stop(bool err) noexcept {
        if(m) m->add(err, elapsed());
      }
      Metric* m;
    };

    Metric() noexcept
      : m_error(0), m_count(0),
        m_min(0), m_max(0), m_total(0) {
    }
    Metric(const Metric&)            = delete;
    Metric(Metric&&)                 = delete;
    Metric& operator=(const Metric&) = delete;
    Metric& operator=(Metric&&)      = delete;

    SWC_CAN_INLINE
    Tracker tracker() noexcept {
      return Tracker(this);
    }

    void add(bool err, uint64_t ns) noexcept;

    void gather(Metric& m) noexcept;

    void reset() noexcept;

    uint24_t m_error;
    uint32_t m_count;
    uint64_t m_min;
    uint64_t m_max;
    uint64_t m_total;
  };

  Statistics(bool enabled) noexcept : enabled(enabled), fds_count(0) { }
  Statistics(const Statistics&)            = delete;
  Statistics(Statistics&&)                 = delete;
  Statistics& operator=(const Statistics&) = delete;
  Statistics& operator=(Statistics&&)      = delete;

  SWC_CAN_INLINE
  Metric::Tracker tracker(Command cmd) noexcept {
    return enabled ? metrics[cmd].tracker() : Metric::Tracker();
  }

  void gather(Statistics& stats) noexcept;

  void reset() noexcept;

  const bool              enabled;
  Metric                  metrics[Command::MAX];
  Core::Atomic<uint64_t>  fds_count;

};


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Statistics.cc"
#endif


#endif // swcdb_fs_Statistics_h
