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
            Property::V_GINT32::Ptr ram_percent, 
            Property::V_GINT32::Ptr ram_release_rate,
            const std::function<size_t(size_t)>& release_call=0)
            : cfg_ram_percent(ram_percent),
              cfg_ram_release_rate(ram_release_rate),
              m_timer(*io), m_release_call(release_call), 
              next_major_chk(-1), ram(MAX_RAM_CHK_INTVAL_MS) {

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
  size_t need_ram() const {
    return ram.used > ram.allowed 
            ? ram.used - ram.allowed 
            : (ram.used_reg > ram.allowed
                ? ram.used_reg - ram.allowed
              : 0);
  }

  SWC_CAN_INLINE 
  size_t avail_ram() const {
    return ram.allowed > ram.used_reg 
            ? (ram.allowed > ram.used ? ram.allowed - ram.used_reg : 0)
            : 0;
  }

  SWC_CAN_INLINE 
  bool need_ram(uint32_t sz) const {
    return ram.free < sz * 2 || 
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
      if(ram.used > ram.allowed) {
        auto inst = MallocExtension::instance();
        inst->SetMemoryReleaseRate(cfg_ram_release_rate->get());
        inst->ReleaseFreeMemory();
        inst->SetMemoryReleaseRate(release_rate_default);
      }
  #endif
  }

  void checker() {
    refresh_stats();

    if(size_t bytes = need_ram()) {
      if(m_release_call) {
        size_t released_bytes = m_release_call(bytes);
        SWC_LOGF(LOG_DEBUG, "Resources::ram release=%lu/%lu %s", 
                  released_bytes, bytes, to_string().c_str());
      }

      malloc_release();
    }

    schedule();
  }

  void refresh_stats() {
    if(!++next_major_chk) {
      page_size = sysconf(_SC_PAGE_SIZE);
    
      ram.total   = page_size * sysconf(_SC_PHYS_PAGES); 
      ram.free    = page_size * sysconf(_SC_AVPHYS_PAGES);
      ram.allowed = (ram.total/100) * cfg_ram_percent->get();
      ram.chk_ms  = ram.allowed / 3000; //~= ram-buff   
      if(ram.chk_ms > MAX_RAM_CHK_INTVAL_MS)
        ram.chk_ms = MAX_RAM_CHK_INTVAL_MS;
    }

    size_t sz = 0, rss = 0;
    std::ifstream buffer("/proc/self/statm");
    buffer >> sz >> rss;
    buffer.close();
    rss *= page_size;
    ram.used = ram.used > ram.allowed || ram.used > rss 
                ? (ram.used + rss) / 2 : rss;
  }

  void schedule() {
    m_timer.expires_from_now(std::chrono::milliseconds(ram.chk_ms));
    m_timer.async_wait(
      [this](const asio::error_code& ec) {
        if(ec != asio::error::operation_aborted) {
          checker();
        }
    }); 
  }

  struct Component final {
    std::atomic<size_t>   total    = 0;
    std::atomic<size_t>   free     = 0;
    std::atomic<size_t>   used     = 0;
    std::atomic<size_t>   used_reg = 0;
    std::atomic<size_t>   allowed  = 0;
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
      s.append(")");
      return s;
    }
  };


  Property::V_GINT32::Ptr             cfg_ram_percent;
  Property::V_GINT32::Ptr             cfg_ram_release_rate;
  asio::high_resolution_timer         m_timer; 
  
  const std::function<size_t(size_t)> m_release_call;
  int8_t                              next_major_chk;

  LockAtomic::Unique                  m_mutex;
  uint32_t                            page_size;
  Component                           ram;
  // Component                     storage;
  
#if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default; 
#endif


};


}
#endif // swc_common_sys_Resources_h
