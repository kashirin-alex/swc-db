/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_sys_Resources_h
#define swc_core_sys_Resources_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/IoContext.h"


namespace SWC { 

class Resources final {
  public:

  Resources();

  ~Resources();

  void init(asio::io_context* io, 
            Property::V_GINT32::Ptr ram_percent, 
            Property::V_GINT32::Ptr ram_release_rate,
            std::function<void(size_t)> release_call=0);

  const size_t need_ram() const;

  const size_t avail_ram() const;

  const bool need_ram(const uint32_t& sz) const;

  void stop();

  const std::string to_string() const;

  private:

  void checker();

  void refresh_stats();

  void schedule(uint32_t ms = 5000);

  struct Component final {
    std::atomic<size_t> total    = 0;
    std::atomic<size_t> free     = 0;
    std::atomic<size_t> used     = 0;
    std::atomic<size_t> allowed  = 0;

    const std::string to_string(uint32_t base = 1) const;
  };


  asio::high_resolution_timer*  m_timer; 
  Property::V_GINT32::Ptr       cfg_ram_percent;
  Property::V_GINT32::Ptr       cfg_ram_release_rate;
  
  std::function<void(size_t)>   release;

  int8_t                        next_major_chk = 0;
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
