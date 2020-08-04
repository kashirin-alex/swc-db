/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_common_sys_Resources_h
#define swc_common_sys_Resources_h

#include <sys/sysinfo.h>
#include <fstream>

#include "swcdb/core/config/Property.h"
#include "swcdb/core/comm/IoContext.h"

#if defined TCMALLOC_MINIMAL || defined TCMALLOC
#include <gperftools/malloc_extension.h>
#endif

namespace SWC { 

class Resources final {
  static const uint32_t MAX_RAM_CHK_INTVAL_MS = 5000;
  
  public:
  
  Resources(asio::io_context* io, 
            Property::V_GINT32::Ptr ram_percent_allowed,
            Property::V_GINT32::Ptr ram_percent_reserved,
            Property::V_GINT32::Ptr ram_release_rate,
            const std::function<size_t(size_t)>& release_call=0)
            : cfg_ram_percent_allowed(ram_percent_allowed),
              cfg_ram_percent_reserved(ram_percent_reserved),
              cfg_ram_release_rate(ram_release_rate),
              m_timer(*io), m_release_call(release_call), 
              next_major_chk(99), ram(MAX_RAM_CHK_INTVAL_MS) {

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
  bool is_low_mem_state() const {
    return ram.free < ram.reserved;
  }

  SWC_CAN_INLINE 
  size_t need_ram() const {
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
  size_t avail_ram() const {
    return !is_low_mem_state() && ram.allowed > ram.used_reg 
            ? (ram.allowed > ram.used ? ram.allowed - ram.used_reg : 0)
            : 0;
  }

  SWC_CAN_INLINE 
  bool need_ram(uint32_t sz) const {
    return is_low_mem_state() || ram.free < sz * 2 || 
           (ram.used_reg + sz > ram.allowed || ram.used + sz > ram.allowed);
  }

  void adj_mem_usage(ssize_t sz) {
    if(sz) {
      m_mutex.lock();
      if(sz < 0 && ram.used_reg < size_t(-sz))
        ram.used_reg = 0;
      else 
        ram.used_reg += sz;
      m_mutex.unlock();
    }
  }

  void more_mem_usage(size_t sz) {
    if(sz) {
      m_mutex.lock();
      ram.used_reg += sz;
      m_mutex.unlock();
    }
  }

  void less_mem_usage(size_t sz) {
    if(sz) {
      m_mutex.lock();
      if(ram.used_reg < sz)
        ram.used_reg = 0;
      else 
        ram.used_reg -= sz;
      m_mutex.unlock();
    }
  }

  SWC_CAN_INLINE 
  uint32_t concurrency() const {
    return m_concurrency;
  }

  SWC_CAN_INLINE 
  uint32_t available_cpu_mhz() const {
    return m_cpu_mhz;
  }

  SWC_CAN_INLINE 
  uint32_t available_mem_mb() const {
    return (ram.total - ram.reserved) / 1024 / 1024;
  }

  void stop() {
    m_timer.cancel();
  }

  std::string to_string() const {
    std::string s("Resources(");
    s.append("Mem-MB-");
    s.append(ram.to_string(1048576));
    s.append(")");
    return s;
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
  #endif
  }

  void checker() {
    refresh_stats();

    bool try_release;
    size_t bytes;
    if((try_release = is_low_mem_state()))
      bytes = 0;
    else
      try_release = (bytes = need_ram());

    if(try_release) {
      if(m_release_call) {
        size_t released_bytes = m_release_call(bytes);
        SWC_LOGF(LOG_DEBUG, "%s mem-released=%lu/%lu", 
                  to_string().c_str(), released_bytes, bytes);
      }

      malloc_release();
    }

    schedule();
  }

  void refresh_stats() {
    if(++next_major_chk % (is_low_mem_state() ? 10 : 100) == 0) {
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
             memcmp(tmp.c_str(), "MemTotal:", 9) == 0) {
            metric = &ram.total;
          } else 
          if(tmp.length() == 13 && 
              memcmp(tmp.c_str(), "MemAvailable:", 13) == 0) {
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
      ram.allowed = (ram.total/100) * cfg_ram_percent_allowed->get();
      ram.reserved = (ram.total/100) * cfg_ram_percent_reserved->get();

      ram.chk_ms  = ram.allowed / 3000; //~= ram-buff   
      if(ram.chk_ms > MAX_RAM_CHK_INTVAL_MS)
        ram.chk_ms = MAX_RAM_CHK_INTVAL_MS;
      
      if(next_major_chk % 100 == 0 && is_low_mem_state())
        SWC_LOGF(LOG_WARN, "Low-Memory state %s", to_string().c_str());
      
      size_t concurrency = !m_concurrency || next_major_chk % 100 == 0
        ? std::thread::hardware_concurrency() : 0;
      if(concurrency) {
        std::ifstream buffer(
          "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq");
        if(buffer.is_open()) {
          size_t khz = 0;
          buffer >> khz;
          buffer.close();
          if(khz) {
            m_cpu_mhz = concurrency * (khz/1000);
            m_concurrency = concurrency;
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
      ram.used = ram.used > ram.allowed || ram.used > rss
                  ? (ram.used + rss) / 2 : rss;
    }
  }

  void schedule() {
    m_timer.expires_from_now(std::chrono::milliseconds(ram.chk_ms));
    m_timer.async_wait(
      [this](const asio::error_code& ec) {
        if(ec == asio::error::operation_aborted)
          return;
        try {
          checker();
        } catch(const std::exception& e) {
          SWC_LOGF(LOG_ERROR, "Resources:checker what=(%s) %s", 
                   e.what(), to_string().c_str());
          schedule();
        }
    }); 
  }

  struct Component final {
    typedef std::atomic<size_t> Bytes;
    Bytes   total    = 0;
    Bytes   free     = 0;
    Bytes   used     = 0;
    Bytes   used_reg = 0;
    Bytes   allowed  = 0;
    Bytes   reserved = 0;
    std::atomic<uint32_t> chk_ms   = 0;

    Component(uint32_t ms): chk_ms(ms) { }

    std::string to_string(uint32_t base = 1) const {
      std::string s("Res(");
      s.append("total=");
      s.append(std::to_string(total/base));
      s.append(" free=");
      s.append(std::to_string(free/base));
      s.append(" used=");
      s.append(std::to_string(used_reg/base));
      s.append("/");
      s.append(std::to_string(used/base));
      s.append(" allowed=");
      s.append(std::to_string(allowed/base));  
      s.append(" reserved=");
      s.append(std::to_string(reserved/base));      
      s.append(")");
      return s;
    }
  };


  Property::V_GINT32::Ptr             cfg_ram_percent_allowed;
  Property::V_GINT32::Ptr             cfg_ram_percent_reserved;
  Property::V_GINT32::Ptr             cfg_ram_release_rate;
  asio::high_resolution_timer         m_timer; 
  
  const std::function<size_t(size_t)> m_release_call;
  int8_t                              next_major_chk;

  LockAtomic::Unique                  m_mutex;
  uint32_t                            page_size;
  Component                           ram;
  // Component                     storage;
  
  std::atomic<uint32_t>               m_concurrency = 0;
  std::atomic<uint32_t>               m_cpu_mhz = 0;
  
#if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default; 
#endif


};


}
#endif // swc_common_sys_Resources_h
