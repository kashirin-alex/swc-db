/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_sys_Resources_h
#define swcdb_common_sys_Resources_h


#include "swcdb/core/config/Property.h"
#include "swcdb/core/comm/IoContext.h"

#include <sys/sysinfo.h>
#include <fstream>


#if defined TCMALLOC_MINIMAL || defined TCMALLOC
#include <gperftools/malloc_extension.h>

#elif defined MIMALLOC
#include <mimalloc.h>

#endif


namespace SWC { namespace Common {

class Resources final {
  static const uint32_t MAX_RAM_CHK_INTVAL_MS = 5000;

  public:

  Resources(const Comm::IoContextPtr& ioctx,
            Config::Property::V_GINT32::Ptr ram_percent_allowed,
            Config::Property::V_GINT32::Ptr ram_percent_reserved,
            Config::Property::V_GINT32::Ptr ram_release_rate,
            const std::function<size_t(size_t)>& release_call=nullptr)
            : cfg_ram_percent_allowed(ram_percent_allowed),
              cfg_ram_percent_reserved(ram_percent_reserved),
              cfg_ram_release_rate(ram_release_rate),
              m_timer(ioctx->executor()),
              m_release_call(release_call),
              next_major_chk(99),
              ram(MAX_RAM_CHK_INTVAL_MS),
              m_concurrency(0), m_cpu_mhz(0) {

#if defined TCMALLOC_MINIMAL || defined TCMALLOC
    release_rate_default = MallocExtension::instance()->GetMemoryReleaseRate();
    if(release_rate_default < 0)
      release_rate_default = 0;
#endif

    checker();
  }

  Resources(const Resources& other) = delete;

  Resources(const Resources&& other) = delete;

  Resources operator=(const Resources& other) = delete;

  ~Resources() { }

  SWC_CAN_INLINE
  bool is_low_mem_state() const noexcept {
    return ram.free < ram.reserved;
  }

  SWC_CAN_INLINE
  size_t need_ram() const noexcept {
    return is_low_mem_state()
            ? ram.used_reg.load()
            : (ram.used > ram.allowed
                ? ram.used - ram.allowed
                : (ram.used_reg > ram.allowed
                    ? ram.used_reg - ram.allowed
                    : 0
                  )
              );
  }

  SWC_CAN_INLINE
  size_t avail_ram() const noexcept {
    return !is_low_mem_state() && ram.allowed > ram.used_reg
            ? (ram.allowed > ram.used ? ram.allowed - ram.used_reg : 0)
            : 0;
  }

  SWC_CAN_INLINE
  bool need_ram(uint32_t sz) const noexcept {
    return is_low_mem_state() || ram.free < sz * 2 ||
           (ram.used_reg + sz > ram.allowed || ram.used + sz > ram.allowed);
  }

  void adj_mem_usage(ssize_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      if(sz < 0 && ram.used_reg < size_t(-sz))
        ram.used_reg.store(0);
      else
        ram.used_reg.fetch_add(sz);
      m_mutex.unlock();
    }
  }

  void more_mem_usage(size_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      ram.used_reg.fetch_add(sz);
      m_mutex.unlock();
    }
  }

  void less_mem_usage(size_t sz) noexcept {
    if(sz) {
      m_mutex.lock();
      if(ram.used_reg < sz)
        ram.used_reg.store(0);
      else
        ram.used_reg.fetch_sub(sz);
      m_mutex.unlock();
    }
  }

  SWC_CAN_INLINE
  uint32_t concurrency() const noexcept {
    return m_concurrency;
  }

  SWC_CAN_INLINE
  uint32_t available_cpu_mhz() const noexcept {
    return m_cpu_mhz;
  }

  SWC_CAN_INLINE
  uint32_t available_mem_mb() const noexcept {
    return (ram.total - ram.reserved) / 1024 / 1024;
  }

  void stop() {
    m_timer.cancel();
  }

  void print(std::ostream& out) const {
    out << "Resources(";
    ram.print(out << "Mem-MB-", 1048576);
    out << ')';
  }

  private:

  void malloc_release() {
  #if defined TCMALLOC_MINIMAL || defined TCMALLOC
    if(ram.used > ram.allowed || is_low_mem_state()) {
      auto inst = MallocExtension::instance();
      inst->SetMemoryReleaseRate(cfg_ram_release_rate->get());
      inst->ReleaseFreeMemory();
      inst->SetMemoryReleaseRate(release_rate_default);
    }

  #elif defined MIMALLOC
    if(ram.used > ram.allowed || is_low_mem_state()) {
      mi_collect(true);
    }
  #endif
  }

  void checker() {
    refresh_stats();

    size_t bytes = 0;
    if(is_low_mem_state() || (bytes = need_ram())) {
      if(m_release_call) {
        size_t released_bytes = m_release_call(bytes);
        SWC_LOG_OUT(LOG_DEBUG,
          print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM
            << " mem-released=" << released_bytes << '/' << bytes;
        );
      }

      malloc_release();
    }

    schedule();
  }

  void refresh_stats() {
    if(!(++next_major_chk % (is_low_mem_state() ? 10 : 100))) {
      page_size = sysconf(_SC_PAGE_SIZE);

      std::ifstream buffer("/proc/meminfo");
      if(buffer.is_open()) {
        size_t sz = 0;
        std::string tmp;
        uint8_t looking(2);
        Component::Bytes* metric(nullptr);
        do {
          buffer >> tmp;
          if(tmp.length() == 9 &&
             !memcmp(tmp.c_str(), "MemTotal:", 9)) {
            metric = &ram.total;
          } else
          if(tmp.length() == 13 &&
             !memcmp(tmp.c_str(), "MemAvailable:", 13)) {
            metric = &ram.free;
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
      }
      /*
      ram.total   = page_size * sysconf(_SC_PHYS_PAGES);
      ram.free    = page_size * sysconf(_SC_AVPHYS_PAGES);
      */
      ram.allowed.store((ram.total/100) * cfg_ram_percent_allowed->get());
      ram.reserved.store((ram.total/100) * cfg_ram_percent_reserved->get());

      ram.chk_ms.store(ram.allowed / 3000); //~= ram-buff
      if(ram.chk_ms > MAX_RAM_CHK_INTVAL_MS)
        ram.chk_ms.store(MAX_RAM_CHK_INTVAL_MS);

      if(!(next_major_chk % 100) && is_low_mem_state())
        SWC_LOG_OUT(LOG_WARN, print(SWC_LOG_OSTREAM << "Low-Memory state "););

      size_t concurrency = !m_concurrency || !(next_major_chk % 100)
        ? std::thread::hardware_concurrency() : 0;
      if(concurrency) {
        std::ifstream buffer(
          "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq");
        if(buffer.is_open()) {
          size_t khz = 0;
          buffer >> khz;
          buffer.close();
          if(khz) {
            m_cpu_mhz.store(concurrency * (khz/1000));
            m_concurrency.store(concurrency);
          }
        } else {
          std::ifstream buffer("/proc/cpuinfo");
          if(buffer.is_open()) {
            size_t mhz = 0;
            std::string tmp;
            size_t tmp_speed = 0;
            do {
              buffer >> tmp;
              if(!memcmp(tmp.c_str(), "cpu", 3)) {
                buffer >> tmp;
                if(!memcmp(tmp.c_str(), "MHz", 3)) {
                  buffer >> tmp >> tmp_speed;
                  mhz += tmp_speed;
                }
              }
            } while (!buffer.eof());
            buffer.close();
            if(mhz) {
              m_cpu_mhz.store(mhz);
              m_concurrency.store(concurrency);
            }
          }
        }
      }
    }

    std::ifstream buffer("/proc/self/statm");
    if(buffer.is_open()) {
      size_t sz = 0, rss = 0;
      buffer >> sz >> rss;
      buffer.close();
      rss *= page_size;
      ram.used.store(ram.used > ram.allowed || ram.used > rss
                      ? (ram.used + rss) / 2 : rss);
    }
  }

  void schedule() {
    m_timer.expires_after(std::chrono::milliseconds(ram.chk_ms));
    m_timer.async_wait(
      [this](const asio::error_code& ec) {
        if(ec == asio::error::operation_aborted)
          return;
        try {
          checker();
        } catch(...) {
          const Error::Exception& e = SWC_CURRENT_EXCEPTION("Resources:checker");
          SWC_LOG_OUT(LOG_ERROR,
            print(SWC_LOG_OSTREAM);
            SWC_LOG_OSTREAM << e;
          );
          schedule();
        }
    });
  }

  struct Component final {
    typedef Core::Atomic<size_t> Bytes;
    Bytes   total;
    Bytes   free;
    Bytes   used;
    Bytes   used_reg;
    Bytes   allowed;
    Bytes   reserved;
    Core::Atomic<uint32_t> chk_ms;

    Component(uint32_t ms=0) noexcept
              : total(0), free(0), used(0), used_reg(0),
                allowed(0), reserved(0), chk_ms(ms) { }

    void print(std::ostream& out, size_t base = 1) const {
      out << "Res("
          << "total=" << (total/base)
          << " free=" << (free/base)
          << " used=" << (used_reg/base) << '/' << (used/base)
          << " allowed=" << (allowed/base)
          << " reserved=" << (reserved/base)
          << ')';
    }
  };


  Config::Property::V_GINT32::Ptr     cfg_ram_percent_allowed;
  Config::Property::V_GINT32::Ptr     cfg_ram_percent_reserved;
  Config::Property::V_GINT32::Ptr     cfg_ram_release_rate;
  asio::high_resolution_timer         m_timer;

  const std::function<size_t(size_t)> m_release_call;
  int8_t                              next_major_chk;

  Core::MutexAtomic                   m_mutex;
  uint32_t                            page_size;
  Component                           ram;
  // Component                     storage;

  Core::Atomic<uint32_t>              m_concurrency;
  Core::Atomic<uint32_t>              m_cpu_mhz;

#if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default;
#endif


};


}}


#endif // swcdb_common_sys_Resources_h
