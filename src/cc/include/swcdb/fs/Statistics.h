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


  struct Metric : Core::MutexAtomic {

    struct Tracker : Time::Measure_ns {
      Tracker(Metric* m) noexcept;
      Tracker(Tracker&& other) noexcept
        : Time::Measure_ns(std::move(*this)), m(other.m.exchange(nullptr)) {
      }
      Tracker(const Tracker&)            = delete;
      Tracker& operator=(const Tracker&) = delete;
      Tracker& operator=(Tracker&&)      = delete;
      ~Tracker() { stop(); }
      void stop() noexcept;

      std::atomic<Metric*> m;
    };

    Metric() noexcept : m_count(0), m_min(0), m_max(0), m_total(0) { }
    Metric(const Metric&)            = delete;
    Metric(Metric&&)                 = delete;
    Metric& operator=(const Metric&) = delete;
    Metric& operator=(Metric&&)      = delete;

    SWC_CAN_INLINE
    Tracker tracker() noexcept {
      return Tracker(this);
    }

    void add(uint48_t ns) noexcept;

    void gather(Metric& m) noexcept;

    void reset() noexcept;

    uint24_t m_count;
    uint48_t m_min;
    uint48_t m_max;
    uint64_t m_total;
  };

  Statistics() noexcept { }

  SWC_CAN_INLINE
  Metric::Tracker tracker(Command cmd) noexcept {
    return metrics[cmd].tracker();
  }

  void gather(Statistics& stats) noexcept;

  void reset() noexcept;

  Metric metrics[Command::MAX];

};


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Statistics.cc"
#endif


#endif // swcdb_fs_Statistics_h
