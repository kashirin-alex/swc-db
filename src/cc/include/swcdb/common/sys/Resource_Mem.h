/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_sys_Resources_Mem_h
#define swcdb_common_sys_Resources_Mem_h


#if defined(__MINGW64__) || defined(_WIN32)
#include <sysinfoapi.h>
#else
#include <sys/sysinfo.h>
#include <fstream>
#endif


#if defined TCMALLOC_MINIMAL || defined TCMALLOC
#include <gperftools/malloc_extension.h>

#elif defined MIMALLOC
#include <mimalloc.h>

#elif defined JEMALLOC
#include <jemalloc/jemalloc.h>

#else
#include <malloc.h>

#endif



namespace SWC { namespace System {


class Mem {
  static const uint32_t FAIL_MS           = 10;
  static const uint32_t BASE_MS           = 250;
  static const uint32_t URGENT_MS         = 100;
  static const uint32_t MAX_MS            = 5000;
  static const uint32_t RELEASE_NORMAL_MS = 500;
  static const uint32_t RELEASE_URGENT_MS = 200;

  public:
  typedef Mem*                          Ptr;
  typedef std::function<size_t(size_t)> ReleaseCall_t;
  Config::Property::Value_int32_g::Ptr  cfg_percent_allowed;
  Config::Property::Value_int32_g::Ptr  cfg_percent_reserved;
  Config::Property::Value_int32_g::Ptr  cfg_release_rate;

  Core::Atomic<size_t>   free;
  Core::Atomic<size_t>   used;
  Core::Atomic<size_t>   used_reg;
  Core::Atomic<size_t>   used_releasable;
  Core::Atomic<size_t>   used_future;
  Core::Atomic<size_t>   total;
  Core::Atomic<size_t>   allowed;
  Core::Atomic<size_t>   reserved;
  Core::Atomic<size_t>   mem_buff_ms;

  Mem(Config::Property::Value_int32_g::Ptr ram_percent_allowed,
      Config::Property::Value_int32_g::Ptr ram_percent_reserved,
      Config::Property::Value_int32_g::Ptr ram_release_rate,
      Notifier* a_notifier,
      ReleaseCall_t&& a_release_call=nullptr) noexcept
      : cfg_percent_allowed(ram_percent_allowed),
        cfg_percent_reserved(ram_percent_reserved),
        cfg_release_rate(ram_release_rate),
        free(0), used(0), used_reg(0), used_releasable(0), used_future(0),
        total(0), allowed(0), reserved(0), mem_buff_ms(0),
        notifier(a_notifier), release_call(std::move(a_release_call)),
        chk_stat_ts(0), chk_statm_ts(0), chk_low_state_ts(0),
        chk_release_ts(0) {

    #if !defined(__MINGW64__) && !defined(_WIN32)
    page_size = sysconf(_SC_PAGE_SIZE);
    #endif

    #if defined TCMALLOC_MINIMAL || defined TCMALLOC
    release_rate_default = MallocExtension::instance()->GetMemoryReleaseRate();
    if(release_rate_default < 0)
      release_rate_default = 0;
    #else
    (void)cfg_release_rate->get(); // in-case unused
    #endif
  }

  ~Mem() noexcept { }

  uint64_t check(uint64_t ts) noexcept {
    try {
      if(ts >= (chk_stat_ts + (is_low_mem_state() ? URGENT_MS
                                                  : MAX_MS))) {
        _check_malloc(ts);
        if(!_check_stat())
          return FAIL_MS;
        chk_stat_ts = ts;
      }

      if(ts >= (chk_statm_ts + (is_low_mem_state() ? URGENT_MS
                                                   : mem_buff_ms))) {
        _check_malloc(ts);
        if(!_check_statm())
          return FAIL_MS;
        chk_statm_ts = ts;
      }

      if(ts >= (chk_release_ts + (is_low_mem_state() ? RELEASE_URGENT_MS
                                                     : RELEASE_NORMAL_MS))) {
        _check_malloc(ts);
        _release();
        chk_release_ts = ts;
      }

      _check_malloc(ts);

      if(is_low_mem_state() && chk_low_state_ts <= ts) {
        chk_low_state_ts = ts + 10000;
        SWC_LOG_OUT(
          LOG_WARN,
          print(SWC_LOG_OSTREAM << "Low-Memory state ");
        );
      }
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      return FAIL_MS;
    }
    return BASE_MS;
  }


  SWC_CAN_INLINE
  bool is_low_mem_state() const noexcept {
    return free < reserved;
  }

  SWC_CAN_INLINE
  size_t need_ram() const noexcept {
    ssize_t need = reserved;
    need -= free;
    if(need < 0) {
      need = used;
      need -= allowed;
      if(need < 0) {
        need = used_reg;
        need += used_future;
        need -= allowed;
        if(need < 0)
          return 0;
      }
    }
    ssize_t _releasable = used_releasable;
    return _releasable > need ? need : _releasable;
  }

  SWC_CAN_INLINE
  size_t avail_ram() const noexcept {
    if(is_low_mem_state() || used > allowed)
      return 0;
    ssize_t _allowed = allowed;
    _allowed -= used_reg;
    _allowed -= used_future;
    return _allowed > 0 ? _allowed : 0;
  }

  SWC_CAN_INLINE
  bool need_ram(uint32_t sz) const noexcept {
    return is_low_mem_state() ||
           free < (sz * 2) ||
           (used_reg + used_future + sz) > allowed ||
           (used + sz) > allowed;
  }


  SWC_CAN_INLINE
  void more_mem_future(size_t sz) noexcept {
    used_future.fetch_add(sz);
  }
  SWC_CAN_INLINE
  void less_mem_future(size_t sz) noexcept {
    used_future.fetch_sub(sz);
    /*
    size_t was = used_future.fetch_sub(sz);
    if(sz > was)
      SWC_LOGF(LOG_WARN,
        "less_mem_future OVERFLOW was=" SWC_FMT_LU " less=" SWC_FMT_LU,
        was, sz);
    */
  }


  SWC_CAN_INLINE
  void more_mem_releasable(size_t sz) noexcept {
    used_releasable.fetch_add(sz);
  }
  SWC_CAN_INLINE
  void less_mem_releasable(size_t sz) noexcept {
    used_releasable.fetch_sub(sz);
    /*
    size_t was = used_releasable.fetch_sub(sz);
    if(sz > was)
      SWC_LOGF(LOG_WARN,
        "less_mem_releasable OVERFLOW was=" SWC_FMT_LU " less=" SWC_FMT_LU,
        was, sz);
    */
  }
  SWC_CAN_INLINE
  void adj_mem_releasable(ssize_t sz) noexcept {
    used_releasable.fetch_add(sz);
    /*
    size_t was = used_releasable.fetch_add(sz);
    if(sz < 0 && size_t(-sz) > was)
      SWC_LOGF(LOG_WARN,
        "adj_mem_releasable OVERFLOW was=" SWC_FMT_LU " adj=" SWC_FMT_LD,
        was, sz);
    */
  }


  SWC_CAN_INLINE
  uint32_t available_mem_mb() const noexcept {
    return (total - reserved) / 1024 / 1024;
  }

  void print(std::ostream& out, size_t base = 1048576) const {
    out << "Res(total=" << (total/base)
        << " free=" << (free/base)
        << " bytes-used=" << used_releasable << '/' << used_reg << '/' << used
        << " allowed=" << (allowed/base)
        << " reserved=" << (reserved/base) << ')';
  }

  private:

  bool _check_stat() {
    if(notifier)
      notifier->rss_used_reg(used_reg);

    #if defined(__MINGW64__) || defined(_WIN32)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    if(!GlobalMemoryStatusEx (&statex) || !statex.ullTotalPhys || !statex.ullAvailPhys)
      return false;
    total.store(statex.ullTotalPhys);
    free.store(statex.ullAvailPhys);

    #else
    std::ifstream buffer("/proc/meminfo");
    if(!buffer.is_open())
      return false;

    size_t sz = 0;
    std::string tmp;
    uint8_t looking(2);
    Core::Atomic<size_t>* metric(nullptr);
    do {
      buffer >> tmp;
      if(tmp.length() == 9 &&
         Condition::str_eq(tmp.c_str(), "MemTotal:", 9)) {
        metric = &total;
      } else
      if(tmp.length() == 13 &&
         Condition::str_eq(tmp.c_str(), "MemAvailable:", 13)) {
        metric = &free;
      }
      if(metric) {
        buffer >> sz >> tmp;
        metric->store(
          sz * (tmp.front() == 'k'
                ? 1024
                : (tmp.front() == 'm' ? 1048576 : 0))
        );
        metric = nullptr;
        --looking;
      }
    } while (looking && !buffer.eof());
    buffer.close();
    #endif

    size_t _allowed = (total/100) * cfg_percent_allowed->get();
    allowed.store(_allowed);
    reserved.store((total/100) * cfg_percent_reserved->get());

    _allowed /= 3000;
    mem_buff_ms.store(_allowed > MAX_MS ? MAX_MS : _allowed);

    if(notifier)
      notifier->rss_free(free);
    return true;
  }

  bool _check_statm() {
    size_t rss = 0;
    #if defined(__MINGW64__) || defined(_WIN32)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    if(!GlobalMemoryStatusEx (&statex) || !statex.ullTotalPhys || !statex.ullAvailPhys)
      return false;
    rss = statex.ullTotalPhys - statex.ullAvailPhys;

    #else
    std::ifstream buffer("/proc/self/statm");
    if(!buffer.is_open())
      return false;
    size_t sz = 0;
    buffer >> sz >> rss;
    buffer.close();
    rss *= page_size;
    #endif

    used.store(used > allowed || used > rss ? (used + rss) / 2 : rss);
    if(notifier)
      notifier->rss_used(rss);
    return true;
  }

  bool _check_malloc(uint64_t ts) {
    size_t bytes;
    #if defined TCMALLOC_MINIMAL || defined TCMALLOC
      // without metadata_bytes
      if(!MallocExtension::instance()->GetNumericProperty(
        "generic.current_allocated_bytes", &bytes))
          return false;
      /*
      MallocExtension::TCMallocStats stats;
      MallocExtension::instance()->ExtractStats(&stats, nullptr, nullptr, nullptr);
      bytes = stats.pageheap.system_bytes;
      bytes += stats.metadata_bytes;
      // bytes =~ rss
      bytes -= stats.thread_bytes;
      bytes -= stats.central_bytes;
      bytes -= stats.transfer_bytes;
      bytes -= stats.pageheap.free_bytes;
      bytes -= stats.pageheap.unmapped_bytes;
      */

    /*
    #elif defined MIMALLOC
      auto h = mi_heap_get_default();
      stats = h->tld->stats;
    */

    #elif defined JEMALLOC
      if(mallctl("epoch", NULL, NULL, &ts, sizeof(ts)))
        return false;
      size_t _bytes;
      size_t sz = sizeof(size_t);
      if(mallctl("stats.active", &_bytes, &sz, NULL, 0))
        return false;
      bytes = _bytes;
      if(mallctl("stats.metadata", &_bytes, &sz, NULL, 0))
        return false;
      bytes += _bytes;

    #elif defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33
      struct mallinfo2 mi;
      mi = ::mallinfo2();
      bytes = mi.uordblks;

    #elif defined(__MINGW64__) || defined(_WIN32)
      MEMORYSTATUSEX statex;
      statex.dwLength = sizeof (statex);
      if(!GlobalMemoryStatusEx (&statex) || !statex.ullTotalPhys || !statex.ullAvailPhys)
        return false;
      bytes = statex.ullTotalPhys - statex.ullAvailPhys;

    #elif defined(SWC_ENABLE_SANITIZER)
      bytes = used;

    #else
      struct mallinfo mi;
      static_assert(
        sizeof(mi.uordblks) == sizeof(size_t),
        "Unsupported mallinfo size-type"
      );
      mi = ::mallinfo();
      bytes = mi.uordblks;
    #endif

    used_reg.store(bytes);
    (void)ts;
    return true;
  }

  void _release() {
    size_t bytes = release_call ? need_ram() : 0;
    if(!bytes && !is_low_mem_state())
      return;

    if(bytes) {
      size_t released = release_call(bytes);
      SWC_LOG_OUT(LOG_DEBUG,
        print(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << " mem-released=" << released << '/' << bytes;
      );
    }

    #if defined TCMALLOC_MINIMAL || defined TCMALLOC
      if(used > allowed || is_low_mem_state()) {
        auto inst = MallocExtension::instance();
        inst->SetMemoryReleaseRate(cfg_release_rate->get());
        inst->ReleaseFreeMemory();
        inst->SetMemoryReleaseRate(release_rate_default);
      }

    #elif defined MIMALLOC
      if(used > allowed || is_low_mem_state()) {
        mi_collect(true);
      }

    #elif defined JEMALLOC
      // ...

    #elif defined(__MINGW64__) || defined(_WIN32)
      if(used > allowed || is_low_mem_state())
        _heapmin();

    #else
      ssize_t keep = reserved;
      keep -= free;
      if(keep > 0) {
        keep = allowed - keep;
      } else if(used > allowed) {
        keep = allowed;
      }
      if(used > allowed || is_low_mem_state())
        ::malloc_trim(keep > 0 ? keep : 0);

    #endif
  }

  Notifier*           notifier;
  const ReleaseCall_t release_call;

  #if !defined(__MINGW64__) && !defined(_WIN32)
  uint32_t            page_size;
  #endif

  uint64_t            chk_stat_ts;
  uint64_t            chk_statm_ts;
  uint64_t            chk_low_state_ts;
  uint64_t            chk_release_ts;

  #if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default;
  #endif

};


}}

#endif // swcdb_common_sys_Resources_Mem_h
