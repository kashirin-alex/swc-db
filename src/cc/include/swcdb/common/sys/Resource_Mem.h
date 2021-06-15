/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_sys_Resources_Mem_h
#define swcdb_common_sys_Resources_Mem_h


#include <sys/sysinfo.h>
#include <fstream>


#if defined TCMALLOC_MINIMAL || defined TCMALLOC
#include <gperftools/malloc_extension.h>

#elif defined MIMALLOC
#include <mimalloc.h>

#endif



namespace SWC { namespace System {


class Mem {
  static const uint32_t FAIL_MS   = 10;
  static const uint32_t URGENT_MS = 100;
  static const uint32_t MAX_MS    = 5000;

  public:
  typedef Mem*                          Ptr;
  typedef std::function<size_t(size_t)> ReleaseCall_t;
  Config::Property::V_GINT32::Ptr       cfg_percent_allowed;
  Config::Property::V_GINT32::Ptr       cfg_percent_reserved;
  Config::Property::V_GINT32::Ptr       cfg_release_rate;

  Core::Atomic<size_t>   free;
  Core::Atomic<size_t>   used;
  Core::Atomic<size_t>   used_reg;
  Core::Atomic<size_t>   total;
  Core::Atomic<size_t>   allowed;
  Core::Atomic<size_t>   reserved;

  Mem(Config::Property::V_GINT32::Ptr ram_percent_allowed,
      Config::Property::V_GINT32::Ptr ram_percent_reserved,
      Config::Property::V_GINT32::Ptr ram_release_rate,
      Notifier* notifier,
      ReleaseCall_t&& release_call=nullptr) noexcept
      : cfg_percent_allowed(ram_percent_allowed),
        cfg_percent_reserved(ram_percent_reserved),
        cfg_release_rate(ram_release_rate),
        free(0), used(0), used_reg(0), total(0), allowed(0), reserved(0),
        notifier(notifier), release_call(std::move(release_call)),
        page_size(sysconf(_SC_PAGE_SIZE)),
        stat_chk(0), ts_low_state(0) {

    #if defined TCMALLOC_MINIMAL || defined TCMALLOC
    release_rate_default = MallocExtension::instance()->GetMemoryReleaseRate();
    if(release_rate_default < 0)
      release_rate_default = 0;
    #else
    (void)cfg_release_rate->get(); // in-case unused
    #endif
  }

  uint64_t check(uint64_t ts) noexcept {
    try {
      uint64_t ms = _check(ts);
      release();
      return ms;
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      return FAIL_MS;
    }
  }


  SWC_CAN_INLINE
  bool is_low_mem_state() const noexcept {
    return free < reserved;
  }

  SWC_CAN_INLINE
  size_t need_ram() const noexcept {
    return is_low_mem_state()
            ? used_reg.load()
            : (used > allowed
                ? used - allowed
                : (used_reg > allowed
                    ? used_reg - allowed
                    : 0
                  )
              );
  }

  SWC_CAN_INLINE
  size_t avail_ram() const noexcept {
    return !is_low_mem_state() && allowed > used_reg
            ? (allowed > used ? allowed - used_reg : 0)
            : 0;
  }

  SWC_CAN_INLINE
  bool need_ram(uint32_t sz) const noexcept {
    return is_low_mem_state() || free < sz * 2 ||
           (used_reg + sz > allowed || used + sz > allowed);
  }

  SWC_CAN_INLINE
  void adj_mem_usage(ssize_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      if(sz < 0 && used_reg < size_t(-sz))
        used_reg.store(0);
      else
        used_reg.fetch_add(sz);
      m_mutex.unlock();
    }
  }

  SWC_CAN_INLINE
  void more_mem_usage(size_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      used_reg.fetch_add(sz);
      m_mutex.unlock();
    }
  }

  SWC_CAN_INLINE
  uint32_t available_mem_mb() const noexcept {
    return (total - reserved) / 1024 / 1024;
  }

  SWC_CAN_INLINE
  void less_mem_usage(size_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      if(used_reg < sz)
        used_reg.store(0);
      else
        used_reg.fetch_sub(sz);
      m_mutex.unlock();
    }
  }

  void print(std::ostream& out, size_t base = 1048576) const {
    out << "Res(total=" << (total/base)
            << " free=" << (free/base)
            << " used=" << (used_reg/base) << '/' << (used/base)
            << " allowed=" << (allowed/base)
            << " reserved=" << (reserved/base) << ')';
  }

  private:

  uint64_t _check(uint64_t ts) {
    if(notifier)
      notifier->rss_used_reg(used_reg.load());

    if(!total || !free || !stat_chk ||
       stat_chk + (is_low_mem_state() ? URGENT_MS : MAX_MS) <= ts) {
      std::ifstream buffer("/proc/meminfo");
      if(!buffer.is_open())
        return FAIL_MS;

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

      allowed.store((total/100) * cfg_percent_allowed->get());
      reserved.store((total/100) * cfg_percent_reserved->get());

      stat_chk = ts;
      if(notifier)
        notifier->rss_free(free.load());
    }

    std::ifstream buffer("/proc/self/statm");
    if(!buffer.is_open())
      return FAIL_MS;
    size_t sz = 0, rss = 0;
    buffer >> sz >> rss;
    buffer.close();
    rss *= page_size;
    used.store(used > allowed || used > rss ? (used + rss) / 2 : rss);
    if(notifier)
      notifier->rss_used(rss);

    if(is_low_mem_state()) {
      if(ts_low_state + 10000 < ts) {
        ts_low_state = ts;
        SWC_LOG_OUT(LOG_WARN, print(SWC_LOG_OSTREAM << "Low-Memory state "););
      }
      return URGENT_MS;
    }
    uint64_t ms = allowed / 3000; // ~= mem-bw-buffer
    return ms > MAX_MS ? MAX_MS : ms;
  }

  void release() {
    size_t bytes = 0;
    if(is_low_mem_state() || (bytes = need_ram())) {
      if(release_call) {
        size_t released_bytes = release_call(bytes);
        SWC_LOG_OUT(LOG_DEBUG,
          print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM
            << " mem-released=" << released_bytes << '/' << bytes;
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
      #endif
    }
  }

  Notifier*           notifier;
  const ReleaseCall_t release_call;
  uint32_t            page_size;
  uint64_t            stat_chk;
  uint64_t            ts_low_state;
  Core::MutexAtomic   m_mutex;

  #if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default;
  #endif

};


}}

#endif // swcdb_common_sys_Resources_Mem_h
