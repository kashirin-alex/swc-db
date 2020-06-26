/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_sys_Resources_h
#define swc_core_sys_Resources_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/IoContext.h"


namespace SWC { 

class Resources final {
  public:
  static const uint32_t MAX_RAM_CHK_INTVAL_MS = 5000;
  Resources();

  ~Resources();

  void init(asio::io_context* io, 
            Property::V_GINT32::Ptr ram_percent, 
            Property::V_GINT32::Ptr ram_release_rate,
            const std::function<size_t(size_t)>& release_call=0);

  size_t need_ram() const;

  size_t avail_ram() const;

  bool need_ram(uint32_t sz) const;

  void stop();

  std::string to_string() const;

  private:

  void checker();

  void refresh_stats();

  void schedule();

  struct Component final {
    std::atomic<size_t>   total    = 0;
    std::atomic<size_t>   free     = 0;
    std::atomic<size_t>   used     = 0;
    std::atomic<size_t>   allowed  = 0;
    std::atomic<uint32_t> chk_ms   = MAX_RAM_CHK_INTVAL_MS;

    std::string to_string(uint32_t base = 1) const;
  };


  asio::high_resolution_timer*  m_timer; 
  Property::V_GINT32::Ptr       cfg_ram_percent;
  Property::V_GINT32::Ptr       cfg_ram_release_rate;
  
  std::function<size_t(size_t)> release;

  int8_t                        next_major_chk = -1;
  Component                     ram;
  uint32_t                      page_size;
  // Component                     storage;
  
#if defined TCMALLOC_MINIMAL || defined TCMALLOC
  double release_rate_default;
#endif

};



namespace Env {

extern SWC::Resources Resources;

}


}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/sys/Resources.cc"
#endif 

#endif // swc_core_sys_Resources_h
